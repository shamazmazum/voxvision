voxtrees_ng$target:::traverse-leaf-solid
{
    @c1["Traversed leafs containing solid voxels"] = count ();
}

voxtrees_ng$target:::traverse-leaf-hole
{
    @c2["Traversed leafs containing holes"] = count ();
}

voxtrees_ng$target:::traverse-node-covered
{
    @c3["Covered nodes traversed"] = count ();
}

voxtrees_ng$target:::data-bb-inside-actual-bb
{
    @c3["Data bb placed behind actual bb"] = count ();
}
