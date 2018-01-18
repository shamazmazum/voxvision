provider voxtrees_ng {
    probe empty__node();
    probe leaf__node();
    probe inner__node();

    probe contains__holes();
    probe covered();
    probe fill__ratio (int);

    probe inc__rec__level();
    probe dec__rec__level();


    probe traverse__leaf__solid();
    probe traverse__leaf__hole();
    probe traverse__node__covered();
    probe data__bb__inside__actual__bb();
};
