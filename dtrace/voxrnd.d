voxrnd$target:::pixel-traced
{
    @c1["Pixels traced"] = count();
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
