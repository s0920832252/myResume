$(function () {

    $(".box .text").each(function (i) {
        let len = 140; // 超過140個字以"..."取代
        if ($(this).text().length > len) {
            //            $(this).attr("title", $(this).text());
            var text = $(this).text().substring(0, len - 1) + "  ...";
            //            console.log(text);
            $(this).text(text);

        }
    });
    $(".text").each(function (i) {
        let len = 200; // 超過180個字以"..."取代
        if ($(this).text().length > len) {
            //            $(this).attr("title", $(this).text());
            var text = $(this).text().substring(0, len - 1) + " ...";
            $(this).text(text);
        }
    });
});
