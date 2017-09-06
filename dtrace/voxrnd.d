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
