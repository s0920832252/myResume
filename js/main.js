$(function () {
    var len = 300; // 超過300個字以"..."取代
    $(".text").each(function (i) {
        if ($(this).text().length > len) {
            //            $(this).attr("title", $(this).text());
            var text = $(this).text().substring(0, len - 1) + "...";
            $(this).text(text);
        }
    });
});
