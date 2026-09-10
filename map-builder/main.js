import * as THREE from 'https://cdnjs.cloudflare.com/ajax/libs/three.js/0.186.0/three.module.min.js';
import { TransformGizmo } from './js/gizmos.js';
import {
    fmtExport,
    formatGrid,
    hexToRgb,
    MIN_SIZE,
    rgbToHex,
    snap,
    snapVec,
} from './js/snap.js';
import { ViewportCamera } from './js/viewport-camera.js';

const viewportEl = document.getElementById('viewport');
const objectListEl = document.getElementById('object-list');
const emptyPropsEl = document.getElementById('empty-props');
const propsFormEl = document.getElementById('props-form');
const exportBoxEl = document.getElementById('export-box');

const prop = {
    name: document.getElementById('prop-name'),
    px: document.getElementById('prop-px'),
    py: document.getElementById('prop-py'),
    pz: document.getElementById('prop-pz'),
    sx: document.getElementById('prop-sx'),
    sy: document.getElementById('prop-sy'),
    sz: document.getElementById('prop-sz'),
    color: document.getElementById('prop-color'),
    wire: document.getElementById('prop-wire'),
    colorLabel: document.getElementById('prop-color-label'),
    wireLabel: document.getElementById('prop-wire-label'),
};

const scene = new THREE.Scene();
scene.background = new THREE.Color(0x141418);
scene.fog = new THREE.Fog(0x141418, 80, 220);

const gizmoScene = new THREE.Scene();

const camera = new THREE.PerspectiveCamera(60, 1, 0.05, 2000);
camera.position.set(8, 6, 10);
camera.lookAt(0, 0, 0);

const renderer = new THREE.WebGLRenderer({ antialias: true });
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
renderer.autoClear = false;
viewportEl.appendChild(renderer.domElement);

const hemi = new THREE.HemisphereLight(0xc5d4ff, 0x2a241c, 1.1);
scene.add(hemi);
const sun = new THREE.DirectionalLight(0xffffff, 1.15);
sun.position.set(12, 20, 8);
scene.add(sun);
scene.add(new THREE.AmbientLight(0xffffff, 0.18));

const grid = new THREE.GridHelper(80, 80, 0x4a4a58, 0x2a2a32);
grid.position.y = 0;
scene.add(grid);
const axes = new THREE.AxesHelper(1.5);
axes.position.y = 0.01;
scene.add(axes);

const viewCam = new ViewportCamera(camera, viewportEl);
viewCam.pivot.set(0, 0, 0);
viewCam.orbitDistance = camera.position.distanceTo(viewCam.pivot);

const unitBox = new THREE.BoxGeometry(1, 1, 1);
const edgeGeom = new THREE.EdgesGeometry(unitBox);

let nextId = 1;
const blocks = [];
const meshToBlock = new Map();
let selected = null;
let suppressHistory = false;
const history = [];
let historyIndex = -1;

class Block {
    constructor({
        id = nextId++,
        name = `Block ${id}`,
        pos = { x: 0, y: 0.5, z: 0 },
        size = { x: 1, y: 1, z: 1 },
        color = { r: 180, g: 180, b: 190 },
        wireColor = { r: 40, g: 40, b: 48 },
    } = {}) {
        this.id = id;
        this.name = name;
        this.pos = snapVec(pos);
        this.size = {
            x: Math.max(MIN_SIZE, snap(size.x)),
            y: Math.max(MIN_SIZE, snap(size.y)),
            z: Math.max(MIN_SIZE, snap(size.z)),
        };
        this.color = { ...color };
        this.wireColor = { ...wireColor };

        this.mesh = new THREE.Mesh(
            unitBox,
            new THREE.MeshLambertMaterial({ color: rgbToThree(this.color) }),
        );
        this.wire = new THREE.LineSegments(
            edgeGeom,
            new THREE.LineBasicMaterial({ color: rgbToThree(this.wireColor) }),
        );
        this.wire.renderOrder = 1;
        scene.add(this.mesh, this.wire);
        meshToBlock.set(this.mesh, this);
        this.sync();
    }

    sync() {
        const p = this.pos;
        const s = this.size;
        this.mesh.position.set(p.x, p.y, p.z);
        this.mesh.scale.set(s.x, s.y, s.z);
        this.mesh.material.color.setRGB(
            this.color.r / 255,
            this.color.g / 255,
            this.color.b / 255,
        );
        this.wire.position.copy(this.mesh.position);
        this.wire.scale.copy(this.mesh.scale);
        const highlight = selected === this;
        this.wire.material.color.set(
            highlight ? 0x6c9dff : rgbToThree(this.wireColor),
        );
        this.wire.material.depthTest = !highlight;
        this.wire.renderOrder = highlight ? 2 : 1;
    }

    toGameObject() {
        const p = this.pos;
        const s = this.size;
        const c = this.color;
        const w = this.wireColor;
        return `CreateBlock(registry, {${fmtExport(p.x)}, ${fmtExport(p.y - 0.5)}, ${fmtExport(p.z)}}, {${fmtExport(s.x)}, ${fmtExport(s.y)}, ${fmtExport(s.z)}}, {${c.r}, ${c.g}, ${c.b}, 255}, {${w.r}, ${w.g}, ${w.b}, 255});`;
    }

    serialize() {
        return {
            id: this.id,
            name: this.name,
            pos: { ...this.pos },
            size: { ...this.size },
            color: { ...this.color },
            wireColor: { ...this.wireColor },
        };
    }

    dispose() {
        scene.remove(this.mesh, this.wire);
        meshToBlock.delete(this.mesh);
        this.mesh.material.dispose();
        this.wire.material.dispose();
    }
}

function rgbToThree(c) {
    return new THREE.Color(c.r / 255, c.g / 255, c.b / 255);
}

function snapshot() {
    return {
        blocks: blocks.map((b) => b.serialize()),
        selectedId: selected ? selected.id : null,
        nextId,
    };
}

function pushHistory() {
    if (suppressHistory) return;
    const json = JSON.stringify(snapshot());
    if (history[historyIndex] === json) return;
    history.splice(historyIndex + 1);
    history.push(json);
    historyIndex = history.length - 1;
    if (history.length > 120) {
        history.shift();
        historyIndex--;
    }
}

function restore(json) {
    suppressHistory = true;
    const data = typeof json === 'string' ? JSON.parse(json) : json;
    for (const b of [...blocks]) {
        b.dispose();
    }
    blocks.length = 0;
    selected = null;
    nextId = data.nextId ?? 1;
    for (const rec of data.blocks) {
        const block = new Block(rec);
        if (block.id >= nextId) nextId = block.id + 1;
        blocks.push(block);
    }
    selected = blocks.find((b) => b.id === data.selectedId) ?? null;
    for (const b of blocks) b.sync();
    suppressHistory = false;
    refreshUi();
}

function buildMapFile() {
    return {
        format: 'raylib-map-builder',
        version: 1,
        blocks: blocks.map((b) => b.serialize()),
        nextId,
    };
}

function parseMapFile(raw) {
    const data = typeof raw === 'string' ? JSON.parse(raw) : raw;
    const records = Array.isArray(data) ? data : data?.blocks;
    if (!Array.isArray(records)) {
        throw new Error('JSON must contain a blocks array.');
    }
    for (const rec of records) {
        if (!rec?.pos || !rec?.size) {
            throw new Error('Each block needs pos and size.');
        }
    }
    let id = Number.isFinite(data?.nextId) ? data.nextId : 1;
    for (const rec of records) {
        if (Number.isFinite(rec.id) && rec.id >= id) id = rec.id + 1;
    }
    return {
        blocks: records,
        nextId: id,
        selectedId: data?.selectedId ?? records[0]?.id ?? null,
    };
}

function saveMapJson() {
    const blob = new Blob([JSON.stringify(buildMapFile(), null, 2)], {
        type: 'application/json',
    });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'map.json';
    a.click();
    URL.revokeObjectURL(url);
}

function loadMapJsonText(text) {
    restore(parseMapFile(text));
    history.length = 0;
    historyIndex = -1;
    pushHistory();
}

function undo() {
    if (historyIndex <= 0) return;
    historyIndex--;
    restore(history[historyIndex]);
}

function redo() {
    if (historyIndex >= history.length - 1) return;
    historyIndex++;
    restore(history[historyIndex]);
}

function select(block) {
    selected = block;
    for (const b of blocks) b.sync();
    refreshUi();
}

function applyToSelected(patch, { record = false } = {}) {
    if (!selected) return;
    if (patch.name != null) selected.name = patch.name;
    if (patch.pos) selected.pos = snapVec(patch.pos);
    if (patch.size) {
        selected.size = {
            x: Math.max(MIN_SIZE, snap(patch.size.x)),
            y: Math.max(MIN_SIZE, snap(patch.size.y)),
            z: Math.max(MIN_SIZE, snap(patch.size.z)),
        };
    }
    if (patch.color) selected.color = { ...patch.color };
    if (patch.wireColor) selected.wireColor = { ...patch.wireColor };
    selected.sync();
    refreshProperties({ keepFocus: true });
    refreshObjectList();
    refreshExport();
    if (record) pushHistory();
}

function addBlock(from = null) {
    const base = from
        ? from.serialize()
        : {
              pos: { x: 0, y: 0.5, z: 0 },
              size: { x: 1, y: 1, z: 1 },
              color: { r: 180, g: 180, b: 190 },
              wireColor: { r: 40, g: 40, b: 48 },
          };
    if (from) {
        base.pos = { ...from.pos, x: snap(from.pos.x + from.size.x) };
        delete base.id;
        delete base.name;
    } else if (selected) {
        base.pos = {
            ...selected.pos,
            x: snap(selected.pos.x + selected.size.x),
        };
        base.size = { ...selected.size };
        base.color = { ...selected.color };
        base.wireColor = { ...selected.wireColor };
    }
    const block = new Block(base);
    blocks.push(block);
    select(block);
    pushHistory();
    return block;
}

function deleteSelected() {
    if (!selected) return;
    const index = blocks.indexOf(selected);
    selected.dispose();
    blocks.splice(index, 1);
    select(blocks[Math.min(index, blocks.length - 1)] ?? null);
    pushHistory();
}

function refreshObjectList() {
    objectListEl.innerHTML = '';
    for (const block of blocks) {
        const li = document.createElement('li');
        if (block === selected) li.classList.add('selected');
        const swatch = document.createElement('span');
        swatch.className = 'swatch';
        swatch.style.background = rgbToHex(block.color);
        const name = document.createElement('span');
        name.textContent = block.name;
        li.append(swatch, name);
        li.addEventListener('click', () => select(block));
        li.addEventListener('dblclick', () => frameSelected());
        objectListEl.append(li);
    }
}

function inputsFocused() {
    const el = document.activeElement;
    return el && (el.tagName === 'INPUT' || el.tagName === 'TEXTAREA');
}

function refreshProperties({ keepFocus = false } = {}) {
    if (!selected) {
        emptyPropsEl.classList.remove('hidden');
        propsFormEl.classList.add('hidden');
        return;
    }
    emptyPropsEl.classList.add('hidden');
    propsFormEl.classList.remove('hidden');
    if (keepFocus && inputsFocused()) {
        const active = document.activeElement;
        if (active === prop.name) return;
    }
    if (document.activeElement !== prop.name) prop.name.value = selected.name;
    if (document.activeElement !== prop.px)
        prop.px.value = formatGrid(selected.pos.x);
    if (document.activeElement !== prop.py)
        prop.py.value = formatGrid(selected.pos.y);
    if (document.activeElement !== prop.pz)
        prop.pz.value = formatGrid(selected.pos.z);
    if (document.activeElement !== prop.sx)
        prop.sx.value = formatGrid(selected.size.x);
    if (document.activeElement !== prop.sy)
        prop.sy.value = formatGrid(selected.size.y);
    if (document.activeElement !== prop.sz)
        prop.sz.value = formatGrid(selected.size.z);
    prop.color.value = rgbToHex(selected.color);
    prop.wire.value = rgbToHex(selected.wireColor);
    prop.colorLabel.textContent = rgbToHex(selected.color);
    prop.wireLabel.textContent = rgbToHex(selected.wireColor);
}

function refreshExport() {
    exportBoxEl.value = blocks.map((b) => b.toGameObject()).join('\n');
}

function refreshUi() {
    refreshObjectList();
    refreshProperties();
    refreshExport();
}

function frameSelected() {
    if (!selected) {
        viewCam.frame(new THREE.Vector3(0, 0, 0), 4);
        return;
    }
    const r = Math.max(selected.size.x, selected.size.y, selected.size.z);
    viewCam.frame(
        new THREE.Vector3(selected.pos.x, selected.pos.y, selected.pos.z),
        r,
    );
}

function setTool(mode) {
    gizmo.setMode(mode);
    document
        .getElementById('tool-move')
        .classList.toggle('active', mode === 'move');
    document
        .getElementById('tool-scale')
        .classList.toggle('active', mode === 'scale');
}

const gizmo = new TransformGizmo({
    gizmoScene,
    camera,
    getBlock: () => selected,
    onChange: (patch) => applyToSelected(patch),
    onCommit: () => pushHistory(),
});

const raycaster = new THREE.Raycaster();
const pointer = new THREE.Vector2();
let draggingGizmo = false;
let pointerDown = null;

function viewportRect() {
    return viewportEl.getBoundingClientRect();
}

function setPointer(event) {
    const rect = viewportRect();
    pointer.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
    pointer.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;
}

function pickBlock(event) {
    setPointer(event);
    raycaster.setFromCamera(pointer, camera);
    const hits = raycaster.intersectObjects(
        blocks.map((b) => b.mesh),
        false,
    );
    return hits[0] ? meshToBlock.get(hits[0].object) : null;
}

function resizeRenderer() {
    const rect = viewportRect();
    const w = Math.max(1, rect.width);
    const h = Math.max(1, rect.height);
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
    renderer.setSize(w, h, false);
}

viewportEl.addEventListener('pointerdown', (event) => {
    if (event.button !== 0) return;
    const rect = viewportRect();
    if (gizmo.beginDrag(event, rect)) {
        draggingGizmo = true;
        viewCam.enabled = false;
        viewportEl.setPointerCapture(event.pointerId);
        return;
    }
    pointerDown = {
        x: event.clientX,
        y: event.clientY,
        block: pickBlock(event),
    };
});

viewportEl.addEventListener('pointermove', (event) => {
    const rect = viewportRect();
    if (draggingGizmo) {
        gizmo.updateDrag(event, rect);
        return;
    }
    const hovered = gizmo.pick(event, rect);
    gizmo.setHover(hovered);
    viewportEl.style.cursor = hovered ? 'grab' : 'default';
});

viewportEl.addEventListener('pointerup', (event) => {
    if (event.button !== 0) return;
    if (draggingGizmo) {
        gizmo.endDrag();
        draggingGizmo = false;
        viewCam.enabled = true;
        return;
    }
    if (!pointerDown) return;
    const dx = event.clientX - pointerDown.x;
    const dy = event.clientY - pointerDown.y;
    if (dx * dx + dy * dy < 9) {
        select(pointerDown.block);
    }
    pointerDown = null;
});

document.getElementById('btn-add').addEventListener('click', () => addBlock());
document.getElementById('btn-dup').addEventListener('click', () => {
    if (selected) addBlock(selected);
});
document.getElementById('btn-delete').addEventListener('click', deleteSelected);
document
    .getElementById('tool-move')
    .addEventListener('click', () => setTool('move'));
document
    .getElementById('tool-scale')
    .addEventListener('click', () => setTool('scale'));
const fileLoadEl = document.getElementById('file-load');
const btnSave = document.getElementById('btn-save');
document.getElementById('btn-save').addEventListener('click', () => {
    saveMapJson();
    btnSave.textContent = 'Saved';
    setTimeout(() => {
        btnSave.textContent = 'Save JSON';
    }, 900);
});
document
    .getElementById('btn-load')
    .addEventListener('click', () => fileLoadEl.click());
fileLoadEl.addEventListener('change', async () => {
    const file = fileLoadEl.files?.[0];
    fileLoadEl.value = '';
    if (!file) return;
    try {
        loadMapJsonText(await file.text());
    } catch (err) {
        window.alert(`Could not load map: ${err.message}`);
    }
});
document.getElementById('btn-copy').addEventListener('click', async () => {
    refreshExport();
    try {
        await navigator.clipboard.writeText(exportBoxEl.value);
        document.getElementById('btn-copy').textContent = 'Copied';
        setTimeout(() => {
            document.getElementById('btn-copy').textContent = 'Copy C++';
        }, 900);
    } catch {
        exportBoxEl.select();
    }
});

prop.name.addEventListener('change', () => {
    applyToSelected(
        { name: prop.name.value.trim() || selected.name },
        { record: true },
    );
});
for (const [el, axis, kind] of [
    [prop.px, 'x', 'pos'],
    [prop.py, 'y', 'pos'],
    [prop.pz, 'z', 'pos'],
    [prop.sx, 'x', 'size'],
    [prop.sy, 'y', 'size'],
    [prop.sz, 'z', 'size'],
]) {
    el.addEventListener('change', () => {
        if (!selected) return;
        const next = { ...selected[kind], [axis]: Number(el.value) };
        applyToSelected({ [kind]: next }, { record: true });
    });
}
prop.color.addEventListener('input', () => {
    applyToSelected({ color: hexToRgb(prop.color.value) });
});
prop.color.addEventListener('change', () => pushHistory());
prop.wire.addEventListener('input', () => {
    applyToSelected({ wireColor: hexToRgb(prop.wire.value) });
});
prop.wire.addEventListener('change', () => pushHistory());

window.addEventListener('keydown', (e) => {
    const typing = viewCam.isTyping();
    if ((e.metaKey || e.ctrlKey) && e.code === 'KeyZ') {
        e.preventDefault();
        if (e.shiftKey) redo();
        else undo();
        return;
    }
    if ((e.metaKey || e.ctrlKey) && e.code === 'KeyY') {
        e.preventDefault();
        redo();
        return;
    }
    if ((e.metaKey || e.ctrlKey) && e.code === 'KeyS') {
        e.preventDefault();
        saveMapJson();
        return;
    }
    if ((e.metaKey || e.ctrlKey) && e.code === 'KeyO') {
        e.preventDefault();
        fileLoadEl.click();
        return;
    }
    if (typing) return;
    if (e.code === 'Digit1') setTool('move');
    if (e.code === 'Digit2') setTool('scale');
    if (e.code === 'KeyF') {
        e.preventDefault();
        frameSelected();
    }
    if (e.code === 'Delete' || e.code === 'Backspace') {
        e.preventDefault();
        deleteSelected();
    }
    if ((e.metaKey || e.ctrlKey) && e.code === 'KeyD') {
        e.preventDefault();
        if (selected) addBlock(selected);
    }
    if (e.code === 'Space') e.preventDefault();
});

function setupSplitters() {
    const sidebar = document.getElementById('sidebar');
    const resize = document.getElementById('sidebar-resize');
    const split = document.getElementById('panel-split');
    const outliner = document.getElementById('outliner');

    resize.addEventListener('pointerdown', (e) => {
        const startX = e.clientX;
        const startW = sidebar.getBoundingClientRect().width;
        const move = (ev) => {
            const w = Math.max(
                240,
                Math.min(520, startW - (ev.clientX - startX)),
            );
            sidebar.style.flexBasis = `${w}px`;
            sidebar.style.width = `${w}px`;
            resizeRenderer();
        };
        const up = () => {
            window.removeEventListener('pointermove', move);
            window.removeEventListener('pointerup', up);
        };
        window.addEventListener('pointermove', move);
        window.addEventListener('pointerup', up);
    });

    split.addEventListener('pointerdown', (e) => {
        const startY = e.clientY;
        const startH = outliner.getBoundingClientRect().height;
        const move = (ev) => {
            const h = Math.max(80, startH + (ev.clientY - startY));
            outliner.style.flex = `0 0 ${h}px`;
        };
        const up = () => {
            window.removeEventListener('pointermove', move);
            window.removeEventListener('pointerup', up);
        };
        window.addEventListener('pointermove', move);
        window.addEventListener('pointerup', up);
    });
}

const clock = new THREE.Clock();
function animate() {
    const delta = clock.getDelta();
    viewCam.update(delta);
    gizmo.update();
    renderer.clear();
    renderer.render(scene, camera);
    renderer.clearDepth();
    renderer.render(gizmoScene, camera);
}

window.addEventListener('resize', resizeRenderer);
new ResizeObserver(resizeRenderer).observe(viewportEl);

setupSplitters();
resizeRenderer();

const ground = new Block({
    name: 'Ground',
    pos: { x: 0, y: -0.5, z: 0 },
    size: { x: 12, y: 1, z: 12 },
    color: { r: 72, g: 78, b: 88 },
    wireColor: { r: 30, g: 32, b: 38 },
});
const starter = new Block({
    name: 'Block 1',
    pos: { x: 0, y: 0.5, z: 0 },
    size: { x: 1, y: 1, z: 1 },
    color: { r: 220, g: 90, b: 80 },
    wireColor: { r: 90, g: 30, b: 28 },
});
blocks.push(ground, starter);
select(starter);
pushHistory();

renderer.setAnimationLoop(animate);
