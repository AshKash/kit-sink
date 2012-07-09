xhr = new XMLHttpRequest();
function handleSayHello() {
	document.getElementById('span_result').innerHTML = xhr.responseText;
}

function sayHello() {
	xhr.open("GET", 'http://p2pmsvmx.prin.am.thmulti.com/apache2-default/index.html.en', true);
	xhr.onreadystatechange = handleSayHello;
	xhr.send(null);
}
