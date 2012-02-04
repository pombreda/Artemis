$(function() {

    function quicksort(items) {
        return items;
    }
    
    function mergesort(items) {
        return items;
    }
    
    function insertionsort(items) {
        return items;
    }

    algorithms = {"quicksort": quicksort,
                  "mergesort": mergesort,
                  "insertionsort": insertionsort};

    function do_sort_handler(event) {
        var algorithm_name = event.target.id.substr(3); // remove the "do-" part
        var algorithm_func = algorithms[algorithm_name];
        
        var items = $("#sortable-input")[0].value.split(" ");
        items = algorithm_func(items);
        
        $("#result-" + algorithm_name).html(items.join(" "));
    };

    // Register event handler
    $(".do-sort").live("click", do_sort_handler);
    
    // Dynamically inserted stuff, after the event handler has been registered
    $("#result-mergesort").after("<h3>Insertion-sort</h3><button class=\"do-sort\" id=\"do-insertionsort\">Sort!</button> > <span id=\"result-insertionsort\" style=\"border: 1px solid #000000;\"></span>");

});

var omgtest = 1;

function omgomgtest() {

    var omgtest = 2;

}

omgomgtest();