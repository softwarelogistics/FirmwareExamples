#include <WebServer.h>

extern WebServer *webServer;

const char *loginIndex =
    "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
    "<tr>"
    "<td colspan=2>"
    "<center><font size=4><b>ESP32 Login Page</b></font></center>"
    "<br>"
    "</td>"
    "<br>"
    "<br>"
    "</tr>"
    "<td>Username:</td>"
    "<td><input type='text' size=25 name='userid'><br></td>"
    "</tr>"
    "<br>"
    "<br>"
    "<tr>"
    "<td>Password:</td>"
    "<td><input type='Password' size=25 name='pwd'><br></td>"
    "<br>"
    "<br>"
    "</tr>"
    "<tr>"
    "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
    "</tr>"
    "</table>"
    "</form>"
    "<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='nuviot' && form.pwd.value=='Test1234')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
    "</script>";

const char *serverIndex =    
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
    "</form>"
    "<div id='prg'>progress: 0%</div>"
    "<script>"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!')"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>";

void sendSensorData(){
  webServer->sendHeader("Connection", "close");

  String html = "<html>";
  html += "<head>";
  html += "<title>Pool Controller</title>";
  html += "<script src = \"https://code.jquery.com/jquery-3.1.1.min.js\" integrity = \"sha256-hVVnYaiADRTO2PzUGmuLJr8BLUSjGIZsDYGmIJLv2b8=\" crossorigin = \"anonymous\"></script>";
  html += "<link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\">";
  html += "<script src = \"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\" integrity = \"sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa\" crossorigin = \"anonymous\"></script>";
  html += "</head>";

  html += "<body>";
  html += "<div class=\"container\">";
  html += "<div class=\"row\">";
  html += "  <h1>Pool Valve Status</h1>";
  html += "  <h1>Version 2.0.1</h1>";
  html += "</div>";

  html += "<div class=\"row\">";
  html += "  <div class='col-md-4'>";
  html += "   <div class=\"panel panel-primary\">";
  html += "     <div class=\"panel-heading\">Source</div>";
  html += "     <div class=\"panel-body\" >";
    html += "       <a href=\"/set/source/pool\"    style=\"width:100px;height:64px\" class=\"btn btn-success\" >Pool</a><br /><br />";
    html += "       <a href=\"/set/source/both\"  style=\"width:100px;height:64px\" class=\"btn btn-success\" >Both</a><br /><br />";
    html += "       <a href=\"/set/source/spa\"  style=\"width:100px;height:64px\" class=\"btn btn-success\" >Spa</a><br /><br />";
  html += "     </div>";
  html += "   </div>";
  html += "  </div>";

  html += "  <div class='col-md-4'>";
  html += "   <div class=\"panel panel-primary\">";
  html += "     <div class=\"panel-heading\">Output</div>";
  html += "     <div class=\"panel-body\" >";
    html += "       <a href=\"/set/output/pool\"    style=\"width:100px;height:64px\" class=\"btn btn-success\" >Pool</a><br /><br />";
    html += "       <a href=\"/set/output/both\"  style=\"width:100px;height:64px\" class=\"btn btn-success\" >Both</a><br /><br />";
    html += "       <a href=\"/set/output/spa\"  style=\"width:100px;height:64px\" class=\"btn btn-success\" >Spa</a><br /><br />";  
  html += "     </div>";
  html += "   </div>";
  html += "  </div>";

  html += "  <div class='col-md-4'>";
  html += "   <div class=\"panel panel-primary\">";
  html += "     <div class=\"panel-heading\">Spa</div>";
  html += "     <div class=\"panel-body\" >";
    html += "      <a href=\"/set/spa/jets\"    style=\"width:100px;height:64px\" class=\"btn btn-success\" >Jets</a><br /><br />";
    html += "      <a href=\"/set/spa/both\"  style=\"width:100px;height:64px\" class=\"btn btn-success\" >Both</a><br /><br />";
    html += "      <a href=\"/set/spa/normal\"  style=\"width:100px;height:64px\" class=\"btn btn-success\" >Normal</a><br /><br />";
  html += "     </div>";
  html += "   </div>";
  html += "  </div>";

  html += "</div>";

  html += "</div>";

  html += "</body>";


//  if(jets.getIsMoving() || source.getIsMoving() || output.getIsMoving()) {
    html += "<script>";
    html += " function autoRefresh() { window.location = window.location.href;}";
    html += " setInterval('autoRefresh()', 2000);";
    html += " </script>";
  //}

  html += "</html>";

  webServer->send(200, "text/html", html);
}    