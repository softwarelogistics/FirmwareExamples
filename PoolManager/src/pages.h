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

void sendSensorData(boolean heaterOn, boolean isHeating, IOValues *values, String fwVersion){
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
  html += "  <h1>Pool Heater</h1>";
  html += "  <h1>Version: " + fwVersion +  "</h1>";
  html += "</div>";

  html += "<div class=\"row\">";
  html += "  <h1>Pool Heater</h1>";
  if(heaterOn){
    html += "<h4 style='color:red'>Heater=ON</h4>";
  }
  else {
    html += "<h4>Heater=OFF</h4>";
  }
  html += "</div>";

  html += "LowPressure  :";    
  html += (ioValues.getValue(0 + 8) == "1" ? "OK<br />" : "Warning<br />");
  html += "Flow:";
  html += (ioValues.getValue(1 + 8) == "1" ? "OK<br />" : "Warning<br />");
  html += "High Pressure:";
  html += (ioValues.getValue(3 + 8) == "1" ? "OK<br />" : "Warning<br />");

  html += "<div class=\"row\">";
  html += "  <h1>Currently Heating</h1>";
  if(isHeating){
    html += "<h4 style='color:red'>Heater=ON</h4>";
  }
  else {
    html += "<h4>Heater=OFF</h4>";
  }
  html += "</div>";

  html += "<div class=\"row\">";
  html += "  <div class='col-md-4'>";
  html += "   <div class=\"panel panel-primary\">";
  html += "     <div class=\"panel-heading\">HEATER</div>";
  html += "     <div class=\"panel-body\" >";
  html += "       <a href=\"/set/heater/on\"    style=\"width:100px;height:64px\" class=\"btn btn-success\" >On</a><br /><br />";
  html += "       <a href=\"/set/heater/off\"  style=\"width:100px;height:64px\" class=\"btn btn-success\" >Off</a><br /><br />";
  html += "     </div>";
  html += "   </div>";
  html += "  </div>";
  html += "  </div>";
  html += "</div>";

  html += "<div class=\"row\">";
  html += "  <div class='col-md-4'>";
  html += "     <div class=\"panel-heading\">STATUS ADC</div>";
  html += "     <div class=\"panel-body\" >";  
  html += "     <ol>";
  for(int idx = 0; idx < 8; ++idx)  
    html += "     <li>" + String(values->getValue(idx)) + "</li>";

  html += "     </ol>";
  html += "     </div>";
  html += "  </div>";
  html += "</div>";

  html += "<div class=\"row\">";
  html += "  <div class='col-md-4'>";
  html += "     <div class=\"panel-heading\">STATUS IO</div>";
  html += "     <div class=\"panel-body\" >";  
  html += "     <ol>";
  for(int idx = 8; idx < 16; ++idx)  
    html += "     <li>" + String(values->getValue(idx)) + "</li>";

  html += "     </ol>";
  html += "     </div>";
  html += "  </div>";
  html += "</div>";


  html += "</div>";

  html += "</body>";


//  if(jets.getIsMoving() || source.getIsMoving() || output.getIsMoving()) {
    // html += "<script>";
    // html += " function autoRefresh() { window.location = window.location.href;}";
    // html += " setInterval('autoRefresh()', 2000);";
    // html += " </script>";
  //}

  html += "</html>";

  webServer->send(200, "text/html", html);
}    