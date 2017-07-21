const NativeWrapper = require('bindings')('wrapper');
Object.assign(module.exports, NativeWrapper);

module.exports.play = function play(filename, options) {
    let stop;
    const promise = new Promise((resolve, reject) => {
        try {
            let handle;

            const release = function _release() {
                try {
                    if (handle) {
                        NativeWrapper.release(handle);
                        handle = null;
                    }
                } catch (e) {
                    reject(e);
                }
            };

            stop = function _stop() {
                try {
                    if (handle) {
                        NativeWrapper.stop(handle);
                        release();
                    }
                } catch (e) {
                    reject(e);
                }
            };

            handle = NativeWrapper.play(filename, options || {});

            const timer = setInterval(() => {
                if (!handle) {
                    clearTimeout(timer);
                } else if (!NativeWrapper.getIsPlaying(handle)) {
                    clearTimeout(timer);
                    release();
                    resolve();
                }
            }, 100);
        } catch (e) {
            reject(e);
        }
    });

    promise.stop = stop;

    return promise;
};

module.exports.getDevices = function getDevices() {
    return NativeWrapper.getDevices();
};
