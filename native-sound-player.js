const NativeWrapper = require('bindings')('wrapper');
const fork = require('child_process').fork;
const path = require('path');

Object.assign(module.exports, NativeWrapper);

module.exports.playSync = function playSync(filename, options) {
    return NativeWrapper.play(filename, options || {});
};
module.exports.getDevices = function getDevices() {
    return NativeWrapper.getDevices();
};

const PlayAsync = path.join(__dirname, 'play-async.js');
module.exports.play = function play(filename, options) {
    return new Promise((resolve, reject) => {
        const child = fork(PlayAsync);
        child.send({ args: [filename, options] });
        child.on('message', e => {
            if (e) reject(e);
            else resolve();

            child.kill();
        });
    });
};
