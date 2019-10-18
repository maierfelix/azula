export type MouseEventType =
  | "onmousedown"
  | "onmouseup"
  | "onmousemove";

export type KeyEventType =
  | "onkeydown"
  | "onkeyup";

export type ScrollEventType =
  | "onmousewheel";

export interface WindowOptions {
  width?: number;
  height?: number;
  title?: string;
  useOffscreenRendering?: boolean;
}

export interface ResizeEvent {
  width: number;
  height: number;
}

export interface CursorChangeEvent {
  name: string;
}

export interface ConsoleMessageEvent {
  level: string;
  callee: Function;
  message: string;
  source: string;
  location: {
    line: number;
    column: number;
  }
}

export class Window {
  title: string;
  width: number;
  height: number;

  constructor (options: WindowOptions);

  update (): void;
  flush (): void;
  shouldClose (): boolean;

  loadHTML (html: string): void;
  loadFile (path: string): void;

  onresize: (event: ResizeEvent) => any;
  oncursorchange: (event: CursorChangeEvent) => any;
  onconsolemessage: (event: ConsoleMessageEvent) => any;

  dispatchMouseEvent (type: MouseEventType, x: number, y: number, button: number): void;
  dispatchKeyEvent (type: KeyEventType, keyCode: number): void;
  dispatchScrollEvent (type: ScrollEventType, deltaX: number, deltaY: number): void;

  dispatchBinaryBuffer (buffer: ArrayBuffer, args?: object): void;
  onbinarymessage: (buffer: ArrayBuffer, args?: object) => any;

  getSharedHandleD3D11 (): BigInt;
}
