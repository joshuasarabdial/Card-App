'use strict'

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 
let parserLib = ffi.Library('./libcparse', {
	'getSummaryFromFile': ['string', ['string']],
	'getPropertiesFromFile': ['string', ['string']]
});

app.get('/endpoint', function(req, res) {
  const fileName = req.query.file; 
  var c = parserLib.getSummaryFromFile("uploads/"+fileName);
  res.send(c);
});

app.get('/endpoint2', function(req, res) {
	const fileName = req.query.file;
  var c = parserLib.getPropertiesFromFile("uploads/"+fileName);
  res.send(c);
});

app.get('/uploads', function(req, res) {
	fs.readdir('./uploads', function(err, items) {
		console.log(err);
		if (err == null) {
			items = items.filter(item => !(/(^|\/)\.[^\/\.]/g).test(item));
			res.send(items);
		} else {
			res.send('');
		}
	});
});

//Sample endpoint
app.get('/someendpoint', function(req , res){
  //let c = parserLib.getSummaryFromFile("uploads/testCardMin.vcf");
  //console.log(c);
  res.send({
    //c
    foo: "bar"
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
