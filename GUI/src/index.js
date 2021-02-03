var app = require('electron').remote;
var fs = require('fs');
var child = require('child_process');
var moveFile = require('move-file');
var THREE = require('three');


var configQueue = [];
var results = [];
var isRunning = false;
var randomColors = Array.from({length: 1000}, (_, index) => Math.random()*16777215);

const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );

const renderer = new THREE.WebGLRenderer();
renderer.setSize( window.innerWidth, window.innerHeight );
document.body.appendChild( renderer.domElement );



const formToJSON = elements => [].reduce.call(elements, (data, element) => {
    if(element.name.length > 0){
        data[element.name] = element.value;
    }
    return data;
}, {});

document.getElementById("config_form").addEventListener("submit", (e) => {
    e.preventDefault();
    let data = formToJSON(e.target.elements);
    configQueue.push(data);
    
    refreshQueue();
    if(!isRunning){
        simulation();
    }
}, false);

function refreshQueue(){
    let queueDiv = document.getElementById("queue");
    queueDiv.innerHTML = '';
    
    configQueue.forEach(element => {
        var newDiv = createConfigDiv(element);
        queueDiv.appendChild(newDiv);
    });
}

function refreshResults(){
    let resultsDiv = document.getElementById("results");
    resultsDiv.innerHTML = '';
    
    for(let i = 0; i < results.length; i++){
        var newDiv = createConfigDiv(results[i]);
        newDiv.className = 'result';
        newDiv.id = `result_${i}`;
        newDiv.addEventListener('click', (e) => {
            fs.readFile(`output/result_${i}/board.txt`, (err, data) => {
                if(err){
                    console.error(err);
                    return;
                }
                var size = data.toString().split('\n')[0].split('x').map(Number);
                var states = data.toString().split('\n')[1].split(',').map(Number);
                scene.clear();
                for(let i = 0; i < size[0]; i++){
                    for(let j = 0; j < size[1]; j++){
                        for(let k = 0; k < size[2]; k++){
                            const geometry = new THREE.BoxGeometry();
                            const material = new THREE.MeshBasicMaterial( { color: randomColors[states[((k*size[0]) + j) * size[1] + i]] } );
                            const cube = new THREE.Mesh( geometry, material );
                            cube.position.x = i;
                            cube.position.y = j;
                            cube.position.z = k;
                            scene.add( cube );
                        }
                    }
                }
                camera.position.x = (size[0] + size[1] + size[2]) / 1.5;
                camera.position.y = (size[0] + size[1] + size[2]) / 1.5;
                camera.position.z = (size[0] + size[1] + size[2]) / 1.5;
                camera.lookAt(0, 0, 0);
                renderer.render( scene, camera );
            });
        }, false);
        resultsDiv.appendChild(newDiv);
    }
}

function createConfigDiv(element){
    var newDiv = document.createElement('div');

    var boardDiv = document.createElement('div');
    boardDiv.innerHTML = `Board: ${element['x_size']}x${element['y_size']}x${element['z_size']}, ${element['random_seeds']} random seeds`;
    newDiv.appendChild(boardDiv);

    var MCDiv = document.createElement('div');
    MCDiv.innerHTML = `MC: ${element['MC_iterations']} iterations, kt=${element['MC_kt']}`;
    newDiv.appendChild(MCDiv);

    return newDiv;
}

function simulation(){
    isRunning = true;

    createConfigFile(configQueue[0]);
    child.execFile('CellularAutomaton.exe', async function(err, data){
        if(err){
            console.error(err);
            return;
        }
        fs.writeFileSync('time.txt', data.toString());
        var index = results.length;
        results.push(configQueue[0]);
        configQueue.pop();
        refreshQueue();
        refreshResults();
        fs.mkdirSync(`output/result_${index}`);
        await moveFile('config.txt', `output/result_${index}/config.txt`);
        await moveFile('time.txt', `output/result_${index}/time.txt`);
        await moveFile('board.txt', `output/result_${index}/board.txt`);
        isRunning = false;
        if(configQueue.length > 0){
            simulation();
        }
    });
}

function createConfigFile(data){
    var configString = '';
    for(var key in data){
        configString += key + '=' + data[key] + '\n';
    }

    fs.writeFileSync("config.txt", configString);
}