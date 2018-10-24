var fileName = process.argv[2]; 
console.log("Begin updating version");

var versionRe = new RegExp("([0-9]+)\.([0-9]+)\.([0-9][0-9][0-9][0-9][0-9][0-9])([\w-]*)");
var now = new Date();
var todaysDate = (now.getUTCFullYear()%100).toString().padStart(2, "0") + (now.getMonth()+1).toString().padStart(2, "0") + (now.getUTCDate()).toString().padStart(2, "0");

fs = require('fs')
fs.readFile(fileName, 'utf8', function (err,data) {
  if (err) {
    return console.log(err);
  }
  
  var reMatch = data.match(versionRe);
  if (!reMatch) process.exit();

  var majorVer = parseInt(reMatch[1]);
  var minorVer = parseInt(reMatch[2]);
  minorVer++; // Increment the minor version here

  var newVersion = majorVer.toString() + "." + minorVer.toString() + "." + todaysDate + reMatch[4];

  fs.writeFile (fileName, newVersion, function(err) {
                if (err) throw err;
                console.log(`Finished! New version is ${newVersion}`);
            });
});

    