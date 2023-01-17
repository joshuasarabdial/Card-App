// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
	
	$.ajax({
		type: 'get',
		datatype: 'json',
		url: '/uploads',
		success: function (data) {
			//console.log(data);
			for (var i = 0; i < data.length; i++) {
				$('#fileList').append("<option>"+data[i]+"</option>");
				$.ajax({
					url: '/endpoint',
					type: 'get',
					datatype: 'json',
					data: {
						file: data[i]
					},
					success: function (data2) {
						let ind = JSON.parse(data2);
						$('#fileSummary').append("<tr><td><a href=\"/uploads/"+ind.file
						+"\">"+ind.file+"</a></td><td>"+ind.name+"</td><td>"+ind.opLength
						+"</td></tr>");
					},
					fail: function (error) {
						console.log(error);
					}
				});
			}
		},
		fail: function(error) {
			console.log(error);
		}
	});
	
	document.getElementById("fileList").onchange = function() {changeFunction()};
	
	function changeFunction() {
		var x = document.getElementById("fileList").value;
		$.ajax({
			url: '/endpoint2',
			type: 'get',
			datatype: 'json',
			data: {
				file: x
			},
			success: function (data) {
				let card = JSON.parse(data);
				for (let property of card) {
					$('#fileProperties').append("<tr><td>"+property.number+"</td><td>"
					+property.name+"</td><td>"+property.values+"</td></tr>");
				}			
			},
		fail: function (error) {
			console.log(error);
		}
		});
	}

	document.getElementById("clearBtn").onclick = function() {clearFunction()};
	
	function clearFunction() {
		$('#status').html("");
	}
	
	/*$('#clearBtn').onclick(
		console.log("something");
		//$('#status').html("");
	);*/
});
