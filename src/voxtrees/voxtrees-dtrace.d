provider voxtrees {
    probe leaf__node();
    probe inner__node();
    probe empty__node();

    probe rti__early__exit();
    probe rti__first__subspace();
    probe rti__worst__case();
    probe rti__voxels__traversed(int);
    probe rti__voxels__skipped (int);

    probe fill__ratio (int);
    probe holes (int);

    probe dense__leaf();
    probe dense__dots (int);

    probe dense__insertion();
    probe dense__deletion();
    probe leaf__insertion();
    probe leaf__deletion();
};
