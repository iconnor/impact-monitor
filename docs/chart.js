function initChart() {
    // Set up margins and dimensions
    let container = d3.select("#chart");
    let containerWidth = container.node().getBoundingClientRect().width;
    let containerHeight = container.node().getBoundingClientRect().height;
    console.log('Container width: ' + containerWidth + ' height: ' + containerHeight);


    let margin = { top: 10, right: 20, bottom: 20, left: 30 },
        width = containerWidth - margin.left - margin.right,
        height = containerHeight - margin.top - margin.bottom;

    svg = container.append("svg")
        .attr("width", width + margin.left + margin.right)
        .attr("height", document.getElementById('chart').offsetHeight - margin.top - margin.bottom)
        .append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    xScale = d3.scaleTime().range([0, width]);
    yScaleLinear = d3.scaleLinear().domain([-6, 6]).range([height, 0]);


    // Add X Axis label
    svg.append("text")
        .attr("transform",
            "translate(" + (width / 2) + " ," +
            (height + margin.top + 15) + ")")
        .style("text-anchor", "middle")
        .text("Date");

    let yAxisTicks = [-6, -4, -2, 0, 2, 4, 6]

    // Y Axis
    svg.append("g")
        .call(d3.axisLeft(yScaleLinear).tickValues(yAxisTicks))
        .append("text")
        .attr("fill", "#000")
        .attr("transform", "rotate(-90)")
        .attr("y", 6)
        .attr("dy", "1em")
        .attr("text-anchor", "end")
        .text("Acceleration (g)");

}


function updateChart() {
    // Update scales
    xScale.domain(d3.extent(sensorData, d => d.date));

    // Update lines
    ['x', 'y', 'z', 'm'].forEach(variable => {
        svg.selectAll(".line-" + variable).remove();

        let colors = {
            x: 'blue',
            y: 'red',
            z: 'green',
            m: 'black'
        };

        svg.append("path")
            .datum(sensorData)
            .attr("class", "line-" + variable)
            .attr("fill", "none")
            .attr("stroke", colors[variable])
            .attr("stroke-width", 1.5)
            .attr("d", d3.line()
                .x(d => xScale(d.date))
                .y(d => yScaleLinear(d[variable]))
            );
    });
}

function updateHighScore(magnitude) {
    // Update high score
    if (magnitude > highScore) {
        highScore = magnitude;
        // Round to 2 decimal places
        highScore = Math.round(highScore * 100) / 100;
        const numberElement = document.getElementById('highScore');
        
        // Remove all color classes
        circle.classList.remove('bg-red-500', 'bg-blue-500', 'bg-yellow-500', 'bg-white');

        // Add color class based on highScore value
        if (highScore < 1) {
            circle.classList.add('bg-white');
        } else if (highScore < 3) {
            circle.classList.add('bg-yellow-500');
        } else if (highScore < 5) {
            circle.classList.add('bg-blue-500');
        } else {
            circle.classList.add('bg-red-500');
        }

        numberElement.innerText = highScore;
        circle.classList.add('shake');
            
        setTimeout(function() {
            circle.classList.remove('shake');
        }, 500);
    }
    
}

function saveToCsv() {
    // Take the sensorData array and convert it to CSV
    let csvContent = "data:text/csv;charset=utf-8,";
    csvContent += "date,x,y,z,magnitude\n";
    sensorData.forEach(function(rowArray) {
        let row = rowArray.date + "," + rowArray.x + "," + rowArray.y + "," + rowArray.z + "," + rowArray.m + "\n";
        csvContent += row;
    });
    // Send it to the browser as an attachment
    var encodedUri = encodeURI(csvContent);
    var link = document.createElement("a");
    link.setAttribute("href", encodedUri);
    let date = new Date();
    link.setAttribute("download", "mpu6050-" + date.getTime() + ".csv");
    document.body.appendChild(link); // Required for FF
    link.click();

}