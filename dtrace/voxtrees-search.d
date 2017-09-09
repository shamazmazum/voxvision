BEGIN
{
    self->rti_rec_level = 0;
}

voxtrees$target:::rti-voxels-traversed
{
    @rit_voxel_hit["RTI voxels traversed"] = sum(arg0);
}

voxtrees$target:::rti-voxels-skipped
{
    @rit_voxels_skipped["RTI voxel skipped"] = sum(arg0);
}

/* Broken */
/*
pid$target:libvoxtrees*:vox_ray_tree_intersection:entry
{
    self->rti_rec_level++;
}

pid$target:libvoxtrees*:vox_ray_tree_intersection:entry
/ self->rti_rec_level == 1 /
{
    @rti_calls["RTI calls"] = count();
}

pid$target:libvoxtrees*:vox_ray_tree_intersection:return
{
    self->rti_rec_level--;
}

voxtrees$target:::rti-early-exit
/ self->rti_rec_level == 1 /
{
    @rti_early_exit["RTI early exits"] = count();
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
*/
