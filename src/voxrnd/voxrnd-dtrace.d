provider voxrnd {
    probe blocks__traced();
    probe canceled__prediction();
    probe leaf__misprediction();
    probe ignored__prediction();
    probe block__leafs__changed (int);
    probe raymerge__block();
};
