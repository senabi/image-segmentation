let express = require("express");
const { exec } = require("child_process");
const fs = require("fs");
let multer = require("multer");
let port = 3000;

let app = express();
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

let storage = multer.diskStorage({
  destination: function (req, file, cb) {
    cb(null, "./images");
  },
  filename: function (req, file, cb) {
    cb(null, file.originalname);
  },
});
let upload = multer({ storage: storage });

/*
app.use('/a',express.static('/b'));
Above line would serve all files/folders inside of the 'b' directory
And make them accessible through http://localhost:3000/a.
*/
app.use(express.static(__dirname + "/public"));
app.use("/images", express.static("./images"));

app.post(
  "/upload-single",
  upload.single("img-file"),
  function (req, res, next) {
    // req.file is the `profile-file` file
    // req.body will hold the text fields, if there were any
    console.log(JSON.stringify(req.file));
    let response = '<a href="/">Home</a><br>';
    response += "Files uploaded successfully.<br>";
    // response += `<div></div><br/>`;
    if (req.file.size <= 10000000) {
      //less than 10mb
      response += `<img width="600px" src="${req.file.path}" /><br>`;
    } else {
      response += `<a href="${req.file.path}">${req.file.filename}</a><br>`;
    }
    response += `<hr> <label> Segmentate Image </label> <br/>`;
    // response += `<button onclick="window.location.href='/'">Start Segmentation</button>`;
    response += ` <form action="/segmentate" method="get">
      <input type="hidden" name="file" value=${req.file.path} />
      <input type="number" name="n_cluster" placeholder="n_cluster" required /> <br/>
      <input type="number" name="n_iter" placeholder="n_iter" required /> <br/>
      <button type="submit">Start segmentation</button>
      </form>`;
    return res.send(response);
  }
);

app.get("/segmentate", function (req, res, next) {
  console.log(req.query.file);
  let { file, n_cluster, n_iter } = req.query;
  let response = '<a href="/">Home</a><br>';
  let exec_response = "";
  let mpirun = exec(
    `mpirun --allow-run-as-root --hostfile ./hostfile --mca btl_tcp_if_include ham0 mpi-img-seg ${n_cluster} ${file} ${n_iter}`,
    (error, stdout, stderr) => {
      console.log("stdout:" + stdout);
      console.log("stderr:" + stderr);
      exec_response += `<hr> <label> STDOUT </label> <br/>`;
      exec_response += `<pre> ${stdout} </pre> <br/>`;
      exec_response += `<hr> <label> STDERR </label> <br/>`;
      exec_response += `<pre> ${stderr} </pre> <br/>`;
      if (error !== null) {
        console.log("exec error: " + error);
        exec_response += `<hr> <label> EXEC ERROR </label> <br/>`;
        exec_response += `<pre> ${error} </pre> <br/>`;
      }
    }
  );
  mpirun.on("close", () => {
    let resz = exec(
      `ffmpeg -y -i images/test.jpg -vf scale=1080:-1 images/test_rescaled.jpg`
    );
    resz.on("close", () => {
      response += `<img width="600px" src="${file}" />`;
      response += `<img width="600px" src="images/test_rescaled.jpg" /> <br/>`;
      res.send(response + exec_response);
    });
  });
});

app.post(
  "/upload-multiple",
  upload.array("profile-files", 12),
  function (req, res, next) {
    // req.files is array of `profile-files` files
    // req.body will contain the text fields, if there were any
    console.log(JSON.stringify(req.file));
    let response = '<a href="/">Home</a><br>';
    response += "Files uploaded successfully.<br>";
    for (let i = 0; i < req.files.length; i++) {
      response += `<img src="${req.files[i].path}" /><br>`;
    }

    return res.send(response);
  }
);

app.post("/add-node", function (req, res) {
  console.log("enter function");
  const { Host, HostName, Port, User } = req.body;
  console.log({
    host: Host,
    hostname: HostName,
    port: Port,
    user: User,
  });
  // let child=exec(
  let response = '<a href="/">Home</a><br>';
  let add_node = exec(
    `${process.env.HOME}/add-node.sh -H ${Host} -HN ${HostName} -P ${Port} -U ${User}`,
    (error, stdout, stderr) => {
      console.log("stdout:" + stdout);
      console.log("stderr:" + stderr);
      response += `<hr> <label> STDOUT </label> <br/>`;
      response += `<pre> ${stdout} </pre> <br/>`;
      response += `<hr> <label> STDERR </label> <br/>`;
      response += `<pre> ${stderr} </pre> <br/>`;
      if (error !== null) {
        console.log("exec error: " + error);
        response += `<hr> <label> EXEC ERROR </label> <br/>`;
        response += `<pre> ${error} </pre> <br/>`;
      }
    }
  );
  add_node.on("close", () => {
    fs.readFile(
      `${process.env.HOME}/.ssh/config`,
      "utf8",
      function (err, data) {
        if (err) {
          response += `<hr> <label> ERROR NODES </label> <br/>`;
          response += `<pre> ${err.message} </pre> <br />`;
          console.log(err.message);
        } else {
          response += `<hr> <label> NODES </label> <br/>`;
          response += `<pre> ${data} </pre> <br/>`;
          console.log(data);
        }
        res.send(response);
      }
    );
  });
});

app.listen(port, () =>
  console.log(`Server running on port ${port}!\nClick http://localhost:3000/`)
);
