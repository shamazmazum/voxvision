BEGIN
{
    self->make_tree_rec_level = 0;
    self->rti_rec_level = 0;
}

voxtrees$target:::empty-node
{
    @empty_node["Empty nodes"] = count();
}

voxtrees$target:::inner-node
{
    @inner_node["Inner nodes"] = count();
}

voxtrees$target:::leaf-node
{
    @leaf_node["Leaf nodes"] = count();
    @depth_hist[self->make_tree_rec_level-1] = count();
}

voxtrees$target:::dense-leaf
{
    @dense_node["Dense nodes"] = count();
}

voxtrees$target:::dense-dots
{
    @dense_dots["Dots in dense nodes"] = sum(arg0);
}

voxtrees$target:::leaf-insertion
{
    @leaf_insertion["Insertions in leaf nodes"] = count();
}

voxtrees$target:::leaf-deletion
{
    @leaf_deletion["Deletions from leaf nodes"] = count();
}

voxtrees$target:::dense-insertion
{
    @dense_insertion["Insertions in dense nodes"] = count();
}

voxtrees$target:::dense-deletion
{
    @dense_deletion["Deletions from dense nodes"] = count();
}

voxtrees$target:::make-tree-call
{
    self->make_tree_rec_level++;
}

voxtrees$target:::make-tree-return
{
    self->make_tree_rec_level--;
}

voxtrees$target:::fill-ratio
{
    @fill_ratio_hist["Fill ratio histogram"] = lquantize(arg0, 0, 100, 10);
}

voxtrees$target:::rti-call
{
    self->rti_rec_level++;
}

voxtrees$target:::rti-call
/ self->rti_rec_level == 1 /
{
    @rti_calls["RTI calls"] = count();
}

voxtrees$target:::rti-return
{
    self->rti_rec_level--;
}

voxtrees$target:::rti-early-exit
/ self->rti_rec_level == 1 /
{
    @rti_early_exit["RTI early exits"] = count();
}

voxtrees$target:::rti-voxel-hit
{
    @rit_voxel_hit["RTI voxel hits"] = count();
}

voxtrees$target:::rti-voxels-skipped
{
    @rit_voxels_skipped["RTI voxel skipped"] = sum(arg0);
}

voxtrees$target:::rti-first-subspace
/ self->rti_rec_level == 1 /
{
    @rti_first_subspace["RTI first subspace"] = count();
}

voxtrees$target:::rti-worst-case
/ self->rti_rec_level == 1 /
{
    @rti_worst_case["RTI worst cases"] = count();
}

END
{
    printf ("Voxtrees number-by-depth histogram");
    printa (@depth_hist);
}