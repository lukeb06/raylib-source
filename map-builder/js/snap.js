export const GRID = 0.1;
export const MIN_SIZE = 0.1;

export function snap(value, grid = GRID) {
    return Number((Math.round(value / grid) * grid).toFixed(1));
}

export function snapVec(vec, grid = GRID) {
    return {
        x: snap(vec.x, grid),
        y: snap(vec.y, grid),
        z: snap(vec.z, grid),
    };
}

export function formatGrid(value) {
    return snap(value).toFixed(1);
}

export function rgbToHex(color) {
    const to = (c) =>
        Math.max(0, Math.min(255, Math.round(c)))
            .toString(16)
            .padStart(2, '0');
    return `#${to(color.r)}${to(color.g)}${to(color.b)}`;
}

export function hexToRgb(hex) {
    const n = hex.replace('#', '');
    return {
        r: parseInt(n.slice(0, 2), 16),
        g: parseInt(n.slice(2, 4), 16),
        b: parseInt(n.slice(4, 6), 16),
    };
}

export function fmtExport(n) {
    return `${formatGrid(n)}f`;
}
