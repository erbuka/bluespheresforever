(function() {
	$(() => {
		$("div.bsf-menu button").on("click", function() {
			$(`section#${$(this).attr("bsf-section")}`)[0].scrollIntoView({ behavior : "smooth", block : "start" });
		});
	});
})();