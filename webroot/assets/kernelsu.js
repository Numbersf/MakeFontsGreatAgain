let callbackCounter = 0;
function getUniqueCallbackName(prefix) {
    return `${prefix}_callback_${Date.now()}_${callbackCounter++}`;
}

export function exec(command, options = {}) {
    return new Promise((resolve, reject) => {
        const callbackFuncName = getUniqueCallbackName("exec");
        window[callbackFuncName] = (errno, stdout, stderr) => {
            resolve({ errno, stdout, stderr });
            cleanup(callbackFuncName);
        };
        function cleanup(successName) {
            delete window[successName];
        }
        try {
            if (typeof ksu !== 'undefined') {
                ksu.exec(command, JSON.stringify(options), callbackFuncName);
            } else {
                resolve({ errno: 1, stdout: "", stderr: "ksu is not defined" });
            }
        } catch (error) {
            reject(error);
            cleanup(callbackFuncName);
        }
    });
}

class Stdio {
    constructor() {
        this.listeners = {};
    }
    on(event, listener) {
        if (!this.listeners[event]) {
            this.listeners[event] = [];
        }
        this.listeners[event].push(listener);
    }
    emit(event, ...args) {
        if (this.listeners[event]) {
            this.listeners[event].forEach(listener => listener(...args));
        }
    }
}

export function spawn(command, args = [], options = {}) {
    const child = {
        listeners: {},
        stdout: new Stdio(),
        stderr: new Stdio(),
        stdin: new Stdio(),
        on(event, listener) {
            if (!this.listeners[event]) this.listeners[event] = [];
            this.listeners[event].push(listener);
        },
        emit(event, ...args) {
            if (this.listeners[event]) {
                this.listeners[event].forEach(listener => listener(...args));
            }
        }
    };
    const callbackName = getUniqueCallbackName("spawn");
    window[callbackName] = child;
    child.on("exit", () => delete window[callbackName]);
    try {
        if (typeof ksu !== 'undefined') {
            ksu.spawn(command, JSON.stringify(args), JSON.stringify(options), callbackName);
        } else {
            setTimeout(() => {
                child.emit("error", "ksu is not defined");
                child.emit("exit", 1);
            }, 0);
        }
    } catch (error) {
        child.emit("error", error);
        delete window[callbackName];
    }
    return child;
}

export function toast(message) {
    try {
        if (typeof ksu !== 'undefined') {
            ksu.toast(message);
        } else {
            console.log(message);
        }
    } catch (error) {   
        console.error("Error displaying toast:", error);
    }
}
