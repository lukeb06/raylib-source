import * as THREE from "https://cdnjs.cloudflare.com/ajax/libs/three.js/0.186.0/three.module.min.js";

const LOOK_SENS = 0.005;
const ORBIT_SENS = 0.005;
const PAN_SENS = 0.0022;

export class ViewportCamera {
  constructor(camera, domElement) {
    this.camera = camera;
    this.dom = domElement;
    this.keys = new Set();
    this.rightDown = false;
    this.middleDown = false;
    this.shift = false;
    this.orbitDistance = 12;
    this.pivot = new THREE.Vector3(0, 0, 0);
    this.enabled = true;

    this._onKeyDown = this.onKeyDown.bind(this);
    this._onKeyUp = this.onKeyUp.bind(this);
    this._onPointerDown = this.onPointerDown.bind(this);
    this._onPointerMove = this.onPointerMove.bind(this);
    this._onPointerUp = this.onPointerUp.bind(this);
    this._onWheel = this.onWheel.bind(this);
    this._onContext = (e) => e.preventDefault();
    this._onBlur = () => this.keys.clear();
    this._preventAux = (e) => {
      if (e.button === 1) e.preventDefault();
    };

    window.addEventListener("keydown", this._onKeyDown);
    window.addEventListener("keyup", this._onKeyUp);
    window.addEventListener("blur", this._onBlur);
    domElement.addEventListener("pointerdown", this._onPointerDown);
    domElement.addEventListener("mousedown", this._preventAux);
    window.addEventListener("pointermove", this._onPointerMove);
    window.addEventListener("pointerup", this._onPointerUp);
    domElement.addEventListener("wheel", this._onWheel, { passive: false });
    domElement.addEventListener("contextmenu", this._onContext);
  }

  isTyping() {
    const el = document.activeElement;
    if (!el) return false;
    const tag = el.tagName;
    return tag === "INPUT" || tag === "TEXTAREA" || el.isContentEditable;
  }

  onKeyDown(e) {
    this.shift = e.shiftKey;
    if (!this.enabled || this.isTyping()) return;
    this.keys.add(e.code);
  }

  onKeyUp(e) {
    this.shift = e.shiftKey;
    this.keys.delete(e.code);
  }

  onPointerDown(e) {
    if (!this.enabled) return;
    if (e.button === 2) this.rightDown = true;
    if (e.button === 1) this.middleDown = true;
  }

  onPointerUp(e) {
    if (e.button === 2) this.rightDown = false;
    if (e.button === 1) this.middleDown = false;
  }

  onPointerMove(e) {
    if (!this.enabled) return;
    const dx = e.movementX;
    const dy = e.movementY;

    if (this.rightDown) {
      this.look(dx, dy);
    } else if (this.middleDown && this.shift) {
      this.pan(dx, dy);
    } else if (this.middleDown) {
      this.orbit(dx, dy);
    }
  }

  onWheel(e) {
    if (!this.enabled) return;
    e.preventDefault();
    const factor = e.deltaY > 0 ? 1.12 : 1 / 1.12;
    this.orbitDistance = Math.max(1, Math.min(400, this.orbitDistance * factor));
    const dir = new THREE.Vector3();
    this.camera.getWorldDirection(dir);
    this.camera.position.copy(this.pivot).addScaledVector(dir, -this.orbitDistance);
  }

  look(dx, dy) {
    const euler = new THREE.Euler(0, 0, 0, "YXZ");
    euler.setFromQuaternion(this.camera.quaternion);
    euler.y -= dx * LOOK_SENS;
    euler.x -= dy * LOOK_SENS;
    euler.x = Math.max(-Math.PI / 2 + 0.01, Math.min(Math.PI / 2 - 0.01, euler.x));
    this.camera.quaternion.setFromEuler(euler);
    this.syncPivotFromCamera();
  }

  orbit(dx, dy) {
    const offset = this.camera.position.clone().sub(this.pivot);
    const spherical = new THREE.Spherical().setFromVector3(offset);
    spherical.theta -= dx * ORBIT_SENS;
    spherical.phi += dy * ORBIT_SENS;
    spherical.phi = Math.max(0.05, Math.min(Math.PI - 0.05, spherical.phi));
    spherical.radius = this.orbitDistance;
    this.camera.position.copy(this.pivot).add(new THREE.Vector3().setFromSpherical(spherical));
    this.camera.lookAt(this.pivot);
  }

  pan(dx, dy) {
    const dist = this.orbitDistance;
    const x = new THREE.Vector3();
    const y = new THREE.Vector3();
    x.setFromMatrixColumn(this.camera.matrix, 0);
    y.setFromMatrixColumn(this.camera.matrix, 1);
    const move = x.multiplyScalar(-dx * PAN_SENS * dist).add(y.multiplyScalar(dy * PAN_SENS * dist));
    this.camera.position.add(move);
    this.pivot.add(move);
  }

  syncPivotFromCamera() {
    const dir = new THREE.Vector3();
    this.camera.getWorldDirection(dir);
    this.pivot.copy(this.camera.position).addScaledVector(dir, this.orbitDistance);
  }

  frame(center, radius = 4) {
    this.pivot.copy(center);
    this.orbitDistance = Math.max(3, radius * 3);
    const dir = new THREE.Vector3();
    this.camera.getWorldDirection(dir);
    if (dir.lengthSq() < 1e-6) dir.set(0.5, -0.4, 0.7).normalize();
    this.camera.position.copy(this.pivot).addScaledVector(dir, -this.orbitDistance);
    this.camera.lookAt(this.pivot);
  }

  update(delta) {
    if (!this.enabled || this.isTyping()) return;
    if (this.middleDown) return;

    const speed = (this.shift ? 18 : 7) * delta;
    const cam = this.camera;
    const forward = new THREE.Vector3();
    const right = new THREE.Vector3();
    cam.getWorldDirection(forward);
    forward.y = 0;
    if (forward.lengthSq() > 1e-8) forward.normalize();
    right.setFromMatrixColumn(cam.matrix, 0);
    right.y = 0;
    if (right.lengthSq() > 1e-8) right.normalize();

    if (this.keys.has("KeyW")) cam.position.addScaledVector(forward, speed);
    if (this.keys.has("KeyS")) cam.position.addScaledVector(forward, -speed);
    if (this.keys.has("KeyA")) cam.position.addScaledVector(right, -speed);
    if (this.keys.has("KeyD")) cam.position.addScaledVector(right, speed);
    if (this.keys.has("KeyE") || this.keys.has("Space") || this.keys.has("KeyR")) {
      cam.position.y += speed;
    }
    if (this.keys.has("KeyQ") || this.keys.has("KeyF") || this.keys.has("ControlLeft")) {
      cam.position.y -= speed;
    }

    this.syncPivotFromCamera();
  }
}
