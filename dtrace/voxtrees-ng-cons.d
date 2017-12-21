#pragma D option aggsortkey
#pragma D option aggsortkeypos=1

BEGIN
{
    self->make_tree_rec_level = 0;
}

voxtrees_ng$target:::inc-rec-level
{
    self->make_tree_rec_level++;
}

voxtrees_ng$target:::dec-rec-level
{
    self->make_tree_rec_level--;
}

voxtrees_ng$target:::empty-node
{
    @empty_node["Empty nodes"] = count();
}

voxtrees_ng$target:::leaf-node
{
    @leaf_node["Leaf nodes"] = count();
    @depth_hist[self->make_tree_rec_level-1] = count();
}

voxtrees_ng$target:::inner-node
{
    @inner_node["Inner nodes"] = count();
}

voxtrees_ng$target:::contains-holes
{
    @contains_holes["Node contains holes"] = count();
}

voxtrees_ng$target:::covered
{
    @covered["Covered nodes"] = count();
}

voxtrees_ng$target:::fill-ratio
{
    @fill_ratio_hist["Fill ratio histogram"] = lquantize (arg0, 0, 50, 10);
}

END
{
    printf ("Voxtrees number-by-depth histogram");
    printa (@depth_hist);
}
