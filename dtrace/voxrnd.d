#pragma D option aggsortkey
#pragma D option aggsortkeypos=1

voxrnd$target:::blocks-traced
{
    @c1["Blocks traced"] = count();
}

pid$target:libvoxrnd*:vox_render:entry
{
    @c2["Renderer called"] = count();
}

voxrnd$target:::leaf-misprediction
{
    @c3["Leaf mispredictions"] = count();
}

voxrnd$target:::ignored-prediction
{
    @c4["Ignored&verified predictions"] = count();
}

voxrnd$target:::canceled-prediction
{
    @c5["Blocks without prediction"] = count();
}

voxrnd$target:::block-leafs-changed
{
    @leafs_changed["Number of leafs picked for prediction in block"] = lquantize (arg0, 0, 15, 1);
}

voxrnd$target:::raymerge-block
{
    @c6["Blocks with ray merging"] = count();
}