import * as THREE from "https://cdnjs.cloudflare.com/ajax/libs/three.js/0.186.0/three.module.min.js";
import { MIN_SIZE, snap } from "./snap.js";

const AXIS = {
  x: new THREE.Vector3(1, 0, 0),
  y: new THREE.Vector3(0, 1, 0),
  z: new THREE.Vector3(0, 0, 1),
};

const COLORS = {
  x: 0xff3653,
  y: 0x8adb5e,
  z: 0x2c8fff,
};

function makeMat(color) {
  return new THREE.MeshBasicMaterial({
    color,
    depthTest: false,
    depthWrite: false,
    transparent: true,
    opacity: 1,
    toneMapped: false,
  });
}

function lineClosestAxisT(ray, origin, axis) {
  const d1 = ray.direction;
  const d2 = axis;
  const w = new THREE.Vector3().subVectors(ray.origin, origin);
  const a = d1.dot(d1);
  const b = d1.dot(d2);
  const c = d2.dot(d2);
  const d = d1.dot(w);
  const e = d2.dot(w);
  const denom = a * c - b * b;
  if (Math.abs(denom) < 1e-10) return e / c;
  return (a * e - b * d) / denom;
}

export class TransformGizmo {
  constructor({ gizmoScene, camera, getBlock, onChange, onCommit }) {
    this.scene = gizmoScene;
    this.camera = camera;
    this.getBlock = getBlock;
    this.onChange = onChange;
    this.onCommit = onCommit;
    this.mode = "move";
    this.handles = [];
    this.hover = null;
    this.drag = null;
    this.raycaster = new THREE.Raycaster();
    this.raycaster.params.Line = { threshold: 0.2 };

    this.root = new THREE.Group();
    this.root.renderOrder = 1000;
    this.scene.add(this.root);

    this.buildMove();
    this.buildScale();
  }

  setMode(mode) {
    this.mode = mode;
  }

  buildMove() {
    this.moveGroup = new THREE.Group();
    this.root.add(this.moveGroup);

    for (const axis of ["x", "y", "z"]) {
      const group = new THREE.Group();
      if (axis === "x") group.rotation.z = -Math.PI / 2;
      if (axis === "z") group.rotation.x = Math.PI / 2;

      const mat = makeMat(COLORS[axis]);
      const shaft = new THREE.Mesh(new THREE.CylinderGeometry(0.03, 0.03, 0.72, 10), mat);
      shaft.position.y = 0.5;
      const head = new THREE.Mesh(new THREE.ConeGeometry(0.085, 0.2, 12), mat);
      head.position.y = 0.96;
      const hit = new THREE.Mesh(
        new THREE.CylinderGeometry(0.1, 0.1, 1.05, 8),
        new THREE.MeshBasicMaterial({
          visible: false,
          depthTest: false,
          transparent: true,
        }),
      );
      hit.position.y = 0.55;
      hit.userData = { kind: "move", axis };
      shaft.userData = hit.userData;
      head.userData = hit.userData;
      group.add(shaft, head, hit);
      this.moveGroup.add(group);
      this.handles.push(hit, shaft, head);
    }

    const planes = [
      { axes: "xy", color: 0xffff00, rot: [0, 0, 0], pos: [0.22, 0.22, 0] },
      { axes: "xz", color: 0xff00ff, rot: [Math.PI / 2, 0, 0], pos: [0.22, 0, 0.22] },
      { axes: "yz", color: 0x00ffff, rot: [0, 0, Math.PI / 2], pos: [0, 0.22, 0.22] },
    ];
    for (const p of planes) {
      const mat = makeMat(p.color);
      mat.opacity = 0.35;
      const mesh = new THREE.Mesh(new THREE.PlaneGeometry(0.18, 0.18), mat);
      mesh.rotation.set(...p.rot);
      mesh.position.set(...p.pos);
      mesh.userData = { kind: "move-plane", axes: p.axes };
      this.moveGroup.add(mesh);
      this.handles.push(mesh);
    }
  }

  buildScale() {
    this.scaleGroup = new THREE.Group();
    this.root.add(this.scaleGroup);
    const faces = [
      { axis: "x", sign: 1 },
      { axis: "x", sign: -1 },
      { axis: "y", sign: 1 },
      { axis: "y", sign: -1 },
      { axis: "z", sign: 1 },
      { axis: "z", sign: -1 },
    ];
    this.scaleHandles = [];
    for (const face of faces) {
      const mat = makeMat(COLORS[face.axis]);
      const mesh = new THREE.Mesh(new THREE.BoxGeometry(0.14, 0.14, 0.14), mat);
      mesh.userData = { kind: "scale", axis: face.axis, sign: face.sign };
      this.scaleGroup.add(mesh);
      this.scaleHandles.push(mesh);
      this.handles.push(mesh);
    }
  }

  ndcFromEvent(event, rect) {
    return new THREE.Vector2(
      ((event.clientX - rect.left) / rect.width) * 2 - 1,
      -((event.clientY - rect.top) / rect.height) * 2 + 1,
    );
  }

  pick(event, rect) {
    const block = this.getBlock();
    if (!block) return null;
    this.raycaster.setFromCamera(this.ndcFromEvent(event, rect), this.camera);
    const visible = this.handles.filter((h) => {
      if (this.mode === "move") {
        return h.userData.kind === "move" || h.userData.kind === "move-plane";
      }
      return h.userData.kind === "scale";
    });
    const hits = this.raycaster.intersectObjects(visible, false);
    return hits[0] ? hits[0].object : null;
  }

  beginDrag(event, rect) {
    const object = this.pick(event, rect);
    if (!object) return false;
    const block = this.getBlock();
    this.raycaster.setFromCamera(this.ndcFromEvent(event, rect), this.camera);
    const data = object.userData;
    const origin = new THREE.Vector3(block.pos.x, block.pos.y, block.pos.z);

    if (data.kind === "move") {
      const t = lineClosestAxisT(this.raycaster.ray, origin, AXIS[data.axis]);
      this.drag = {
        kind: "move",
        axis: data.axis,
        startT: t,
        startPos: { ...block.pos },
      };
    } else if (data.kind === "move-plane") {
      const normal = data.axes === "xy" ? AXIS.z : data.axes === "xz" ? AXIS.y : AXIS.x;
      const plane = new THREE.Plane().setFromNormalAndCoplanarPoint(normal, origin);
      const hit = new THREE.Vector3();
      this.raycaster.ray.intersectPlane(plane, hit);
      this.drag = {
        kind: "move-plane",
        axes: data.axes,
        normal,
        startHit: hit.clone(),
        startPos: { ...block.pos },
      };
    } else if (data.kind === "scale") {
      const t = lineClosestAxisT(this.raycaster.ray, origin, AXIS[data.axis]);
      this.drag = {
        kind: "scale",
        axis: data.axis,
        sign: data.sign,
        startT: t,
        startPos: { ...block.pos },
        startSize: { ...block.size },
      };
    }
    return true;
  }

  updateDrag(event, rect) {
    if (!this.drag) return;
    const block = this.getBlock();
    if (!block) return;
    this.raycaster.setFromCamera(this.ndcFromEvent(event, rect), this.camera);
    const origin = new THREE.Vector3(
      this.drag.startPos.x,
      this.drag.startPos.y,
      this.drag.startPos.z,
    );

    if (this.drag.kind === "move") {
      const t = lineClosestAxisT(this.raycaster.ray, origin, AXIS[this.drag.axis]);
      const delta = snap(t - this.drag.startT);
      const pos = { ...this.drag.startPos };
      pos[this.drag.axis] = snap(this.drag.startPos[this.drag.axis] + delta);
      this.onChange({ pos });
    } else if (this.drag.kind === "move-plane") {
      const plane = new THREE.Plane().setFromNormalAndCoplanarPoint(this.drag.normal, origin);
      const hit = new THREE.Vector3();
      if (!this.raycaster.ray.intersectPlane(plane, hit)) return;
      const delta = hit.sub(this.drag.startHit);
      const pos = { ...this.drag.startPos };
      const a = this.drag.axes[0];
      const b = this.drag.axes[1];
      pos[a] = snap(this.drag.startPos[a] + delta[a]);
      pos[b] = snap(this.drag.startPos[b] + delta[b]);
      this.onChange({ pos });
    } else if (this.drag.kind === "scale") {
      const t = lineClosestAxisT(this.raycaster.ray, origin, AXIS[this.drag.axis]);
      const delta = snap(t - this.drag.startT);
      const axis = this.drag.axis;
      const sign = this.drag.sign;
      const startMin = this.drag.startPos[axis] - this.drag.startSize[axis] / 2;
      const startMax = this.drag.startPos[axis] + this.drag.startSize[axis] / 2;
      let min = startMin;
      let max = startMax;
      if (sign > 0) {
        max = snap(startMax + delta);
        if (max < min + MIN_SIZE) max = snap(min + MIN_SIZE);
      } else {
        min = snap(startMin + delta);
        if (min > max - MIN_SIZE) min = snap(max - MIN_SIZE);
      }
      const size = { ...this.drag.startSize };
      const pos = { ...this.drag.startPos };
      size[axis] = snap(max - min);
      pos[axis] = snap((min + max) / 2);
      this.onChange({ pos, size });
    }
  }

  endDrag() {
    if (!this.drag) return false;
    this.drag = null;
    this.onCommit();
    return true;
  }

  setHover(object) {
    for (const handle of this.handles) {
      if (handle.material && handle.material.visible !== false && handle.material.opacity !== 0.35) {
        handle.material.opacity = 1;
      }
      if (handle.material && handle.material.opacity === 0.35) {
        handle.material.opacity = 0.35;
      }
    }
    this.hover = object;
    if (object && object.material && object.material.visible !== false) {
      object.material.opacity = object.userData.kind === "move-plane" ? 0.7 : 0.55;
    }
  }

  update() {
    const block = this.getBlock();
    this.root.visible = !!block;
    if (!block) return;

    const pos = new THREE.Vector3(block.pos.x, block.pos.y, block.pos.z);
    const dist = this.camera.position.distanceTo(pos);
    const gizmoScale = THREE.MathUtils.clamp(dist * 0.09, 0.45, 14);

    this.moveGroup.visible = this.mode === "move";
    this.scaleGroup.visible = this.mode === "scale";
    this.moveGroup.position.copy(pos);
    this.moveGroup.scale.setScalar(gizmoScale);

    const size = block.size;
    const hs = THREE.MathUtils.clamp(dist * 0.025, 0.08, 1.6);
    for (const handle of this.scaleHandles) {
      const { axis, sign } = handle.userData;
      handle.position.set(pos.x, pos.y, pos.z);
      handle.position[axis] += (size[axis] / 2) * sign;
      handle.scale.setScalar(hs / 0.14);
    }
  }
}
