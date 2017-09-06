BEGIN
{
    self->make_tree_rec_level = 0;
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

pid$target:libvoxtrees*:vox_make_tree:entry
{
    self->make_tree_rec_level++;
}

pid$target:libvoxtrees*:vox_make_tree:return
{
    self->make_tree_rec_level--;
}

voxtrees$target:::fill-ratio
{
    @fill_ratio_hist["Fill ratio histogram"] = lquantize(arg0, 0, 100, 10);
}

voxtrees$target:::holes
{
    @holes["Holes in leaf nodes"] = sum(arg0);
}

END
{
    printf ("Voxtrees number-by-depth histogram");
    printa (@depth_hist);
}
