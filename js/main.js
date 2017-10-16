$(function () {
    var len = 260; // 超過260個字以"..."取代
    $(".text").each(function (i) {
        if ($(this).text().length > len) {
            //            $(this).attr("title", $(this).text());
            var text = $(this).text().substring(0, len - 1) + "...";
            $(this).text(text);
        }
    });
});
