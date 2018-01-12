provider voxrnd {
    probe blocks__traced();
    probe canceled__prediction();
    probe leaf__misprediction();
    probe ignored__prediction();
};
