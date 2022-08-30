#include "website_controller.h"
#define TAG "WEBSITE_CONTROLLER_C"
#define CONFIG_EXAMPLE_BASIC_AUTH_USERNAME "abc"
#define CONFIG_EXAMPLE_BASIC_AUTH_PASSWORD "abc"
static httpd_uri_t basic_auth = {
    .uri       = "/basic_auth",
    .method    = HTTP_GET,
    .handler   = basic_auth_get_handler,
};
static httpd_uri_t hello  = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    //Let's pass response string in user
   /* context to demonstrate it's usage */
    .user_ctx  = "<!DOCTYPE html><style>@import url(\'https://fonts.googleapis.com/css?family=Barlow|Patua+One\');\n.settingBackground\n{\n\tbackground-color: #e9e9ed;\n\tmargin-left:0;\n\tmargin-right:0;\n\twidth:50vw;\n\ttext-align: center;\n}\n\n\n\n\ninput[type=range] {\n\tmargin-bottom: 1vw;\n\tmargin-top: 1vw;\n\twidth:48vw;\n  }\n  input[type=range]:focus {\n\toutline: none;\n  }\n  input[type=range]::-webkit-slider-runnable-track {\n\twidth: 100%;\n\theight: 1.7vw;\n\tcursor: pointer;\n\tanimate: 0.2s;\n\n\tbackground: #bcbcbe;\n\tborder-radius: 0.3vw;\n\tborder: 0.1vw solid #000000;\n  }\n  input[type=range]::-webkit-slider-thumb {\n\n\tborder: 0.1vw solid #000000;\n\theight: 2vw;\n\twidth: 2vw;\n\tborder-radius: 0.3vw;\n\tbackground: #0066ff;;\n\tcursor: pointer;\n\t-webkit-appearance: none;\n\n  }\n  input[type=range]:focus::-webkit-slider-runnable-track {\n\tbackground: #a93085;\n  }\n  input[type=range]::-moz-range-track {\n\twidth: 100%;\n\theight: 1.7vw;\n\tcursor: pointer;\n\tanimate: 0.2s;\n\n\tbackground: #bcbcbe;\n\tborder-radius: 0.3vw;\n\tborder: 0.1vw solid #000000;\n  }\n  input[type=range]::-moz-range-thumb {\n\n\tborder: 0.1vw solid #000000;\n\theight: 2vw;\n\twidth: 2vw;\n\tborder-radius: 0.3vw;\n\tbackground: #0066ff;;\n\tcursor: pointer;\n  }\n  input[type=range]::-ms-track {\n\twidth: 100%;\n\theight: 10px;\n\tcursor: pointer;\n\tanimate: 0.2s;\n\tbackground: transparent;\n\tborder-color: transparent;\n\tcolor: transparent;\n  }\n  input[type=range]::-ms-fill-lower {\n\tbackground: #a93099;\n\tborder: 1px solid #000000;\n\tborder-radius: 10px;\n\tbox-shadow: 1px 1px 1px #000000;\n  }\n  input[type=range]::-ms-fill-upper {\n\tbackground: #a93085;\n\tborder: 1px solid #000000;\n\tborder-radius: 10px;\n\tbox-shadow: 1px 1px 1px #000000;\n  }\n  input[type=range]::-ms-thumb {\n\tbox-shadow: 1px 1px 1px #000000;\n\tborder: 1px solid #000000;\n\theight: 2vw;\n\twidth: 2vw;\n\tborder-radius: 1vw;\n\tbackground: #f01d1d;\n\tcursor: pointer;\n  }\n  input[type=range]:focus::-ms-fill-lower {\n\tbackground: #a9308b;\n  }\n  input[type=range]:focus::-ms-fill-upper {\n\tbackground: #a97f30;\n  }\n  \n\nlabel\n{\n\tfloat:left;\n\tmargin-top:auto;\n\tmargin-bottom: auto;\n\tmargin-left:0.3vw;\n\tfont-family:\'Barlow\', sans-serif;;\n\tfont-size: 2vw;\n}\nli > label\n{\n\tcolor:white;\n} \n\nselect\n{\n\theight:3.125vw;\n\twidth:9.375vw;\n\tmargin-bottom: 1vw;\n\tfont-size: 2vw;\n\tfont-family:\'Barlow\', sans-serif;;\n}\nbutton\n{\n\tfont-size: 2.5vw;\n\tcolor:black;\n\twidth: 50vw;\n\theight:3.125vw;\n\tbackground-color: #97f26a;\n}\nbutton:active:hover{\n\tbackground-color: #4e8831;\n}\nbutton:hover\n{\n\tbackground-color: #7ecc56;\n}\n.hidden\n{\n\tdisplay:none;\n}\n.colorBox{\n    margin:0px;\n\tmargin-bottom: 1vw;\n    padding:0px;\n    outline:0px solid transparent;\n\twidth:3.125vw;\n\theight:3.125vw;\n}\n.colorDiv\n{\n\tmargin-left:0px;\n\tmargin-right:0px;\n\twidth:100%;\n}\n\t\tdiv{\n\t\t\t  display: block;\n\t\t\t\tmargin-left: auto;\n\t\t\t\tmargin-right: auto;\n\t\t\t\twidth: 50%;\n\t\t}\n\n figure{\n\t display:inline-block;\n\t width:10vw;\n\t height:20vw;\n\t margin:0;\n\t padding:0;\n\t vertical-align: middle\n }\n figcaption{\n\t\tdisplay:inline-block;\n\t\theight:2vw;\n }\n\t\t*{\n\t\t\tmargin:0;\n\t\t\tpadding:0;\n\t\t\tborder-width:0;\n\t\t}\n\t\tnav{\n\t\t\tposition:fixed;\n\t\t\tleft:0;\n\t\t\ttop:0;\n\t\t\theight:5vh;\n\t\t\twidth:100vw;\n\t\t\tbackground-color:black;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-color:#97f26a;\n\t\t\tborder-width:.4vh;\n\t\t}\n\t\tnav ul{\n\t\t\tlist-style-type:none;\n\t\t\tline-height:5vh;\n\t\t\tmargin:0;\n\t\t\twidth:100vw;\n\t\t\tfont-size:1em;\n\t\t}\n\t\tnav ul li{\n\t\t\tdisplay:inline-block;\n\t\t\ttext-align:center;\n\t\t\tfloat:left;\n\t\t\tmargin:0;\n\t\t\t\n\t\t}\n\t\tnav ul li a{\n\t\t\tdisplay:inline-block;\n\t\t\tcolor:white;\n\t\t\ttext-decoration:none;\n\t\t\twidth:5vh;\n\t\t\theight:5vh;\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\tfont-size:1.5vw;\n\t\t}\n\t\tnav ul li a:hover{\n\t\t\tbackground-color:grey;\n\t\t/*\tborder-top-style:outset;*/\n\t\t\tborder-top-width:1vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-bottom-width:.4vh;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t}\n\t\tnav ul li p{\n\t\t\tdisplay:inline-block;\n\t\t\tcolor:white;\n\t\t\ttext-decoration:none;\n\t\t\twidth:20vw;\n\t\t\theight:5vh;\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\tfont-size:2em;\n\t\t}\n\t\tnav ul li:last-child\n\t\t{\n\t\t\tfloat:right;\n\t\t\tmargin-right:7vw;\n\t\t}\n\t\th1{\n\t\t\tmargin-top:3vh;\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\ttext-align:center;\n\t\t\tfont-size:5em;\n\t\t\twidth:50vw;\n\t\t\tmargin:3vh auto;\n\t\t}\n\t\th2{\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\twidth:50vw;\n\t\t\tfont-size:2em;\n\t\t\tmargin:3vh auto;\n\t\t}\n\t\th3{\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\twidth:50vw;\n\t\t\tfont-size:1.2em;\n\t\t\tmargin:3vh auto;\n\t\t}\n\t\tiframe + a{\n\t\t\tfont-family:\'Barlow\', sans-serif;;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\ttext-indent:4vw;\n\t\t\tdisplay:block;\n\t\t}\n\t\th2 + a{\n\t\t\tfont-family:\'Barlow\', sans-serif;;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\ttext-indent:4vw;\n\t\t\tdisplay:block;\n\t\t}\n\t\tbody > a{\n\t\t\tfont-family:\'Barlow\', sans-serif;;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\ttext-indent:4vw;\n\t\t\tdisplay:block;\n\t\t}\n\t\tp{\n\t\t\tfont-family:\'Barlow\', sans-serif;;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\ttext-indent:4vw;\n\t\t}\n\t\tol{\n\t\t\tfont-family:\'Barlow\', sans-serif;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\tlist-style-type:circle;\n\t\t}\n\t\tli{\n\t\t\tmargin-top:.6vh;\n\t\t}\n\t\t\n\t\timg{\n\t\t\tdisplay:block;\n\t\t\tmargin:3vh auto;\n\t\t\twidth:50vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-top-style:solid;\n\t\t\tborder-top-color:#0066ff;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t\tborder-width:1vh;\n\t\t}\n\t\t.iframeS{\n\t\t\twidth:320;\n\t\t\theight:180;\n\t\t}\n\t\tiframe{\n\t\t\tdisplay:block;\n\t\t\tmargin:3vh auto;\n\t\t\twidth:50vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-top-style:solid;\n\t\t\tborder-top-color:#0066ff;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t\tborder-width:1vh;\n\t\t}\n\t\tvideo{\n\t\t\t\tdisplay:block;\n\t\t\tmargin:3vh auto;\n\t\t\twidth:50vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-top-style:solid;\n\t\t\tborder-top-color:#0066ff;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t\tborder-width:1vh;\n\t\t}\n\t\ttable{\n\t\t\tborder-collapse:collapse;\n\t\t\tmargin:3vh auto;\n\t\t\tuser-select:none;\n\t\t}\n\t\ttd,th{\n\t\t\tborder: .3vh solid black;\n\t\t\ttext-align: left;\n\t\t\tpadding: 1vh;\n\t\t\tfont-family:Verdana;\n\t\t}\n\t\tth{\n\t\t\tcolor:white;\n\t\t\tbackground-color:#97f26a;\t\t\n\t\t}\n\n\t\tbody:not(nav){\n\t\t\tposition:relative;\n\t\t\ttop:7vh;\n\t\t}\n\t\t#active{\n\t\t\tbackground-color:#0066ff;\n\t\t}\n\t\t#active:hover{\n\t\t\tbackground-color:#6699ff;\n\t\t}\n\t\t.ordered{\n\t\t\tlist-style-type:lower-alpha;\n\t\t}\n\t\t.imgS{\n\t\t\twidth:10vw;\n\t\t\theight:10vw;\n\t\t}\n\t\tli .imgS{\n\t\tdisplay:inline;\n\t\t\tmargin-top:3vh ;\n\t\t\tmargin-bottom:3vh;\n\t\t\tmargin-left:auto;\n\t\t\tmargin-right:auto;\n\t\t\twidth:10vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-top-style:solid;\n\t\t\tborder-top-color:#0066ff;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t\tborder-width:1vh;\n\t\t}\n\n</style>\n<script></script>\n\n<!--EDIT-->\n<html>\n<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">\n<link rel=\"stylesheet\" type=\"text/css\" href=\"/static/style.css?v=1\">\n<head>\n\t<title>LED Matrix Control</title>\n</head>\n<body>\n\t<div page-name=\"Color_Palette\">\n\t\t<h1>Color Palette</h1>\n\t\t<select id = \"color_possible\">\n\t\t\t<option value =\"def\">Default</option>\n\t\t\t<option value=\"umd\">UMD</option>\n\t\t\t<option value=\"us\">U.S.</option>\n\t\t\t<option value=\"solid\">Solid</option>\n\t\t</select>\n\t\t\n\t\t<form  action = \"/forb\" target = \"hiddenFrame\" >c1\n\t\t\n\t\t\t<input class = \"hidden\" type=\"text\" name = \"submit_type\" value =\"color_set\" display=\"none\"/>\n\t\t\t<div class = \"colorDiv\"><input  class = \"colorBox\" type = \"color\" id = \"c0\" name = \"c0\"/><input  class = \"colorBox\" type = \"color\" id = \"c1\" name = \"c1\"/><input  class = \"colorBox\" type = \"color\" id = \"c2\" name = \"c2\"/><input  class = \"colorBox\" type = \"color\" id = \"c3\" name = \"c3\"/><input  class = \"colorBox\" type = \"color\" id = \"c4\" name = \"c4\"/><input  class = \"colorBox\" type = \"color\" id = \"c5\" name = \"c5\"/><input  class = \"colorBox\" type = \"color\" id = \"c6\" name = \"c6\"/><input  class = \"colorBox\" type = \"color\" id = \"c7\" name = \"c7\"/><input  class = \"colorBox\" type = \"color\" id = \"c8\" name = \"c8\"/><input  class = \"colorBox\" type = \"color\" id = \"c9\" name = \"c9\"/><input  class = \"colorBox\" type = \"color\" id = \"c10\" name = \"ca\"/><input  class = \"colorBox\" type = \"color\" id = \"c11\" name = \"cs\"/><input  class = \"colorBox\" type = \"color\" id = \"c12\" name = \"cd\"/><input  class = \"colorBox\" type = \"color\" id = \"c13\" name = \"c13\"/><input  class = \"colorBox\" type = \"color\" id = \"c14\" name = \"cg\"/><input  class = \"colorBox\" type = \"color\" id = \"c15\" name = \"cp\"/></div>\n\n\t\t\t<select name = \"display\" id = \"matrix_strip_choice_color\">\n\t\t\t\t<option value =\"0\">Matrix</option>\n\t\t\t\t<option value=\"1\">Strip</option>\n\t\t\t\t<option value=\"2\">Both</option>\n\t\n\t\t\t</select>\t\n\t\t\t\n\n\t\t</form>\n\t\t<button  id = \"color_palette_submit\"><i class = \"fa fa-paper-plane\"></i></button>\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t</div>\n\n\t<div page-name=\"Animation_Settings\">\n\t\t\n\t\t<h1>Animation Settings</h1>\n\n\t\t<form action = \"/echo\" target = \"hiddenFrame\" method = \"post\">\n\t\t\t<input class = \"hidden\" type=\"text\" name = \"submit_type\" value =\"settings_set\" display=\"none\"/>\n\t\t\t<div class = \"settingBackground\"><label for =\"sensitivity\">Sensitivity</label><input type = \"range\" min = \"0\" max = \"254\" step = \"1\"  value = \"5\" id = \"sensitivity\" name = \"sensitivity\" class = \"rangeSlider\"></div>\n\t\t\t<div class = \"settingBackground\"><label for =\"brightness\">Brightness</label><input type = \"range\" min = \"0\" max = \"254\" step = \"1\"  value = \"5\" id = \"brightness\" name = \"brightness\" class = \"rangeSlider\"></div>\n\t\t\t<div class = \"settingBackground\"><label for =\"relax\">Drop Time</label><input type = \"range\" min = \"0\" max = \"254\" step = \"1\"  value = \"5\" id= \"relax\" name = \"relax\" class = \"rangeSlider\"></div>\n\t\t\t<div class = \"settingBackground\"><label for =\"hsl_speed\">Color Change Speed</label><input type = \"range\" min = \"0\" max = \"254\" step = \"1\"  value = \"5\" id= \"hsl_speed\" name = \"hsl_speed\" class = \"rangeSlider\"></div>\n\t\t\t<select name = \"mode\" id = \"mode\">\n\t\t\t\t<option value =\"0\">Normal Vis</option>\n\t\t\t\t<option value=\"1\">Image</option>\n\t\t\t\t<option value=\"2\">Image Vis</option>\n\n\t\t\t</select>\n\t\t\t<select name = \"display\" id = \"matrix_strip_animation_choice\">\n\t\t\t\t<option value =\"2\">Matrix</option>\n\t\t\t\t<option value=\"3\">Strip</option>\n\t\t\t\t<option value=\"2\">Both</option>\n\n\t\t\t</select>\n\t\t\t\n\t\t</form>\n\t\t<button id = \"animation_settings_submit\"><i class = \"fa fa-paper-plane\"></i></button>\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t</div>\n\n\n\t<div page-name=\"Display_Image\">\n\t\t<h1>Display Image</h1>\n\t\t<div class = \"settingBackground\"><label for =\"box_size\">Box Size</label><input type = \"range\" min = \"1\" max = \"64\" step = \"1\"  value = \"5\" id = \"box_size\" name = \"box_size\" class = \"rangeSlider\"></div>\n\t\t<input type=\"file\" id=\"image_upload\">\n\t\t<form action = \"/echo\" target = \"hiddenFrame\" method = \"post\">\n\t\t\t<input class = \"hidden\" type=\"text\" name = \"submit_type\" value =\"image_set\" display=\"none\"/>\n\t\t\t<div class = \"hidden\" id = \"image_data\"><input  class = \"colorBox\" type = \"color\" id = \"c0\" name = \"c0\"/></div>\n\t\t\t<select name = \"display\" id = \"display\">\n\t\t\t\t<option value =\"0\">Matrix</option>\n\t\t\t\t<option value=\"1\">Strip</option>\n\t\t\t\t<option value=\"2\">Both</option>\n\n\t\t\t</select>\n\t\t\t<button onclick =submitForm(e) ontouchstart=submitForm(e) ><i class = \"fa fa-paper-plane\"></i></button>\n\n\t\t</form>\n\n\t\t<div style=\"position: relative;\">\n\t\t\t<canvas style=\"position:absolute;top:0;left:0;z-index: 3;\" id= \"layer1\" width=\"500\" height=\"500\"></canvas>\n\t\t\t<canvas style=\"position:absolute;top:0;left:0;z-index: 2;\"  id= \"layer0\" width=\"500\" height=\"500\"></canvas>\n\t\t\t\n\t\t</div>\n\t\t<canvas style = \"border:solid\" width = \"256\" height = \"256\" id = \"dbg_can\"></canvas>\n\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t\t\n\t</div>\n\n\t<div page-name=\"Scroll_Text\">\n\t\t<h1>Scroll Text</h1>\n\t\t<form action = \"/echo\" target = \"hiddenFrame\" method = \"post\">\n\t\t\t<label for =\"fname\">First nameL</label><input type = \"text\" id = \"fname\" name = \"fname\">\n\t\t\t<label for =\"lname\">Last nameL</label><input type = \"text\" id = \"lname\" name = \"lname\">\n\t\t\t<input type = \"submit\" value = \"Submit\">\n\t\t</form>\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t</div>\n\n\t<div page-name=\"API_Based\">\n\t\t<h1>API Based</h1>\n\t\t<form action = \"/echo\" target = \"hiddenFrame\" method = \"post\">\n\t\t\t<label for =\"fname\">First nameL</label><input type = \"text\" id = \"fname\" name = \"fname\">\n\t\t\t<label for =\"lname\">Last nameL</label><input type = \"text\" id = \"lname\" name = \"lname\">\n\t\t\t<input type = \"submit\" value = \"Submit\">\n\t\t</form>\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t</div>\n\n\t<nav>\n\t\t<ul>\n\t\t\t<li>\n\t\t\t\t<a id=\"active\" page-name = \"Color_Palette\" class = \"nav-button\"><i class=\"fa fa-paint-brush\"></i></a>\n\t\t\t</li>\n\t\t\t<li>\n\t\t\t\t<a page-name = \"Animation_Settings\" class = \"nav-button\"><i class=\"fa fa-cog\"></i></a>\n\t\t\t</li>\n\t\t\t<li>\n\t\t\t\t<a page-name = \"Display_Image\" class = \"nav-button\"><i class=\"fa fa-file-image-o\"></i></a>\n\t\t\t</li>\n\t\t\t<li>\n\t\t\t\t<a page-name = \"Scroll_Text\" class = \"nav-button\"><i class=\"fa fa-file-text-o\"></i></a>\n\t\t\t</li>\n\t\t\t<li>\n\t\t\t\t<a page-name = \"API_Based\" class = \"nav-button\"><i class=\"fa fa-cloud\"></i></a>\n\t\t\t</li> \n\n\n\n\t\t\t<li>\n\t\t\t\t<p id = \"Page_Text\">Color Palette</a>\n\t\t\t</li>\n\t\t</ul>\n\t</nav>\n\n\t<script>\n\t\tfunction arr_concat(a,b)\n\t\t{\n\t\t\tconsole.log(\"HELLO\")\n\t\t\tret = new Uint8Array(a.length+b.length);\n\t\t\tfor(var i = 0; i < a.length;i++)\n\t\t\t{\n\t\t\t\tret[i] = a[i];\n\t\t\t}\n\t\t\tfor(var i = 0; i < b.length;i++)\n\t\t\t{\n\t\t\t\tret[a.length + i] = b[i];\n\t\t\t}\n\t\t\tconsole.log(\"brug\",ret);\n\t\t\treturn ret;\n\t\t}\n\n\t\tfunction componentToHex(c) {\n  let hex = c.toString(16);\n  return hex.length == 1 ? \"0\" + hex : hex;\n}\nfunction rgbToHex(r, g, b) {\n  return \"#\" + componentToHex(r) + componentToHex(g) + componentToHex(b);\n}\n\n\t\tvar umd = [\"#c17d11\",  \n\"#c17d11\",  \n\"#555753\",  \n\"#555753\",  \n\"#a40000\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#eeeeec\",  \n\"#c17d11\",  \n\"#c17d11\",  \n\"#555753\",  \n\"#555753\",  \n\"#a40000\",  \n\"#a40000\", \n\"#eeeeec\", \n\"#eeeeec\",]\n\n\t\tvar default_color = [\"#5c3566\",  \n\"#a900ff\",  \n\"#5100ff\",  \n\"#204a87\",  \n\"#00b1ff\",  \n\"#c17d11\",  \n\"#c4a000\",  \n\"#eeeeec\",  \n\"#555753\",  \n\"#73d216\",  \n\"#4e9a06\",  \n\"#f57900\",  \n\"#ce5c00\",  \n\"#b55151\",  \n\"#ef2929\",  \n\"#a40000\"];\n\n\t\tvar us = [\n\t\t\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",\n\t\t]\n\t\tfunction colorUpdate(e)\n\t\t{\n\t\t\tvar choice = e.srcElement.value;\n\t\t\tvar arr = [];\n\t\t\tif(choice === \"umd\")\n\t\t\t{\n\t\t\t\tarr = umd;\n\t\t\t}\n\t\t\tif(choice === \"def\")\n\t\t\t{\n\t\t\t\tarr = default_color;\n\t\t\t\t//console.log(\"DEF\");\n\t\t\t}\n\t\t\tif(choice=== \"us\")\n\t\t\t{\n\t\t\t\tarr = us;\n\t\t\t}\n\t\t\tif(choice ===\"solid\")\n\t\t\t{\n\t\t\t\tvar cb = document.getElementsByClassName(\"colorDiv\")[0].children;\n\t\t\tfor(var i = 0;i < 16;i++)\n\t\t\t{\n\t\t\t\tcb[i].value = cb[0].value;\n\t\t\t}\n\t\t\t\treturn;\n\t\t\t}\n\t\t\tvar cb = document.getElementsByClassName(\"colorDiv\")[0].children;\n\t\t\tfor(var i = 0;i < 16;i++)\n\t\t\t{\n\t\t\t\tcb[i].value = arr[i];\n\t\t\t}\n\t\t}\n\n\t\tfunction getElementByAttribute(attribute, value)\n\t\t{\n\t\t\treturn document.querySelectorAll(\"[\" + attribute + \"=\" + value + \"]\")[0];\n\t\t}\n\n\t\t\n\n\t\tfunction submitForm(e)\n\t\t{\n\t\t\tsend = new Uint8Array(0);\n\t\t\tdisplay_num = parseInt(document.getElementById(\"display\").value);\n\t\t\tconsole.log(e.srcElement.id);\n\t\t\tmessage_binary = \"\";\n\t\t\tdat = new Uint8Array(4 + 6 * 16);\n\t\t\tdat2= new Uint8Array(0);\n\t\t\tdat3 = new Uint8Array(4 + 8);\n\t\t\tconsole.log(\"CALLING\",parseInt(document.getElementById(\"matrix_strip_choice_color\").value));\n\t\t\tif(e.srcElement.id === \"color_palette_submit\")\n\t\t\t{console.log(\"CALLING\");\n\t\t\t\tcolor_data = \"\";\n\t\t\t\t//dat = new Uint8Array(4 + 6 * 16);\n\t\t\t\tdat[0] = 0;\n\t\t\t\tdat[1] = parseInt(document.getElementById(\"matrix_strip_choice_color\").value);;\n\t\t\t\tdat[2] = 0;\n\t\t\t\tdat[3] = 100;\n\t\t\t\tfor(var i = 0;i < 16;i++)\n\t\t\t\t{\n\t\t\t\t\tcolor = document.getElementById(\"c\" + i).value;\n\t\t\t\t\tarr = (Uint16Array.from([parseInt(color.substring(1,3),16),parseInt(color.substring(3,5),16),parseInt(color.substring(5,7),16)]));\n\n\t\t\t\t\tfor(var j = 0; j < 3;j++)\n\t\t\t\t\t{\n\t\t\t\t\t\tu16 = arr[j];\n\t\t\t\t\t\tdat[4 + 6*i+2*j]=((u16 & (65280 >>> 0)) >>> 8);\n\t\t\t\t\t\tdat[4 + 6*i+2*j + 1] = (u16 & (255 >>> 0));\n\t\t\t\t\t\t\n\t\t\t\t\t}\n\t\t\t\t\t\n\n\t\t\t\t}\n\t\t\t\tconsole.log(display_num);\n\t\t\t\t\tif(display_num === 0 || display_num === 2)\n\t\t\t\t{\n\t\t\t\t\tconsole.log(\"CALLING\");\n\t\t\t\t\tsend = arr_concat(send, dat);\n\t\t\t\t}\n\n\t\t\t\tif(display_num === 1 || display_num === 2)\n\t\t\t\t{\n\t\t\t\t\tconsole.log(\"CALLING\");\n\t\t\t\t\tdat2 = dat.slice();\n\t\t\t\t\tdat2[1] = 1;\n\t\t\t\t\tsend = arr_concat(send, dat2);\n\t\t\t\t}\n\t\t\t\tconsole.log(\"SEND\",send);\n\t\t\t}\n\t\t\tif(e.srcElement.id === \"animation_settings_submit\")\n\t\t\t{\n\t\t\t\tif(parseInt(document.getElementById(\"matrix_strip_animation_choice\").value) == 2)\n\t\t\t\t{\n\t\t\t\t\t//sensitivity brightness relax hsl_speed\n\t\t\t\t\tdat3[0] = 0;\n\t\t\t\t\tdat3[1] = 2;\n\t\t\t\t\tdat3[2] = 0;\n\t\t\t\t\tdat3[3] = 12;\n\n\t\t\t\t\tu16 = Uint16Array.from([parseInt(document.getElementById(\"relax\").value)])[0];\n\n\t\t\t\t\tdat3[4 + 0]=((u16 & (65280 >>> 0)) >>> 8);\n\t\t\t\t\tdat3[4 + 1] = (u16 & (255 >>> 0));\n\n\t\t\t\t\tu16 = Uint16Array.from([parseInt(document.getElementById(\"hsl_speed\").value)])[0];\n\n\t\t\t\t\tdat3[4 + 2]=((u16 & (65280 >>> 0)) >>> 8);\n\t\t\t\t\tdat3[4 + 3] = (u16 & (255 >>> 0));\n\n\t\t\t\t\tu16 = Uint16Array.from([parseInt(document.getElementById(\"brightness\").value)])[0];\n\n\t\t\t\t\tdat3[4 + 4]=((u16 & (65280 >>> 0)) >>> 8);\n\t\t\t\t\tdat3[4 + 5] = (u16 & (255 >>> 0));\n\n\t\t\t\t\tu16 = Uint16Array.from([parseInt(document.getElementById(\"sensitivity\").value)])[0];\n\n\t\t\t\t\tdat3[4 + 6]=((u16 & (65280 >>> 0)) >>> 8);\n\t\t\t\t\tdat3[4 + 7] = (u16 & (255 >>> 0));\n\t\t\t\t}\n\t\t\t\tif(parseInt(document.getElementById(\"matrix_strip_animation_choice\").value) == 3)\n\t\t\t\t{\n\t\t\t\t\t//sensitivity brightness relax hsl_speed\n\t\t\t\t\tdat3[0] = 0;\n\t\t\t\t\tdat3[1] = 3;\n\t\t\t\t\tdat3[2] = 0;\n\t\t\t\t\tdat3[3] = 8;\n\n\n\n\t\t\t\t\tu16 = Uint16Array.from([parseInt(document.getElementById(\"hsl_speed\").value)])[0];\n\n\t\t\t\t\tdat3[4 + 0]=((u16 & (65280 >>> 0)) >>> 8);\n\t\t\t\t\tdat3[4 + 1] = (u16 & (255 >>> 0));\n\n\t\t\t\t\tu16 = Uint16Array.from([parseInt(document.getElementById(\"brightness\").value)])[0];\n\n\t\t\t\t\tdat3[4 + 2]=((u16 & (65280 >>> 0)) >>> 8);\n\t\t\t\t\tdat3[4 + 3] = (u16 & (255 >>> 0));\n\n\n\t\t\t\t}\n\t\t\t\tsend = dat3;\n\t\t\t}\n\t\t\t//if(parseInt(document.getElementById(\"display\").value))\n\t\t\t\n\t\t\tconsole.log(\"SEND\",send);\n\t\t\toptions = {\n\t\t\t\tmethod: \'POST\',\n\t\t\t\tbody: send\n\t\t\t};\n\t\t\tfetch(\"/echo\",options);\n\n\t\t\t//e.srcElement.parentNode.parentNode.submit();\n\t\t}\n\t\tfunction navClick(e)\n\t\t{\n\n\t\t\t\n\t\t\tvar old = document.getElementById(\"active\");\n\t\t\told.id = null;\n\t\t\tconsole.log(e.srcElement.parentNode);\n\t\t\tconsole.log(e.srcElement);\n\t\t\tvar properSource;\n\t\t\tif(e.srcElement.tagName === \"A\")\n\t\t\t{\n\t\t\t\tproperSource = e.srcElement;\n\n\t\t\t}\n\t\t\telse\n\t\t\t{\n\t\t\t\tproperSource = e.srcElement.parentNode;\n\n\t\t\t}\n\t\t\tdocument.getElementById(\"Page_Text\").innerHTML = properSource.getAttribute(\"page-name\").replace(\"_\",\" \");\n\t\t\tproperSource.id = \"active\";\n\n\t\t\tgetElementByAttribute(\"page-name\",old.getAttribute(\"page-name\")).style.display = \"none\";\n\t\t\tgetElementByAttribute(\"page-name\",properSource.getAttribute(\"page-name\")).style.display = \"block\";\n\n\t\t\t\n\t\t}\n\n\t\tfunction imageUpload(e)\n\t\t{\n    if(e.target.files) {\n      let imageFile = e.target.files[0]; //here we get the image file\n      var reader = new FileReader();\n      reader.readAsDataURL(imageFile);\n      reader.onloadend = function (e) {\n        var myImage = new Image(); // Creates image object\n        myImage.src = e.target.result; // Assigns converted image to image object\n        myImage.onload = function(ev) {\n          var myCanvas = document.getElementById(\"layer0\"); // Creates a canvas object\n          var myContext = myCanvas.getContext(\"2d\"); // Creates a contect object\n          myCanvas.width = myImage.width; // Assigns image\'s width to canvas\n          myCanvas.height = myImage.height; // Assigns image\'s height to canvas\n          myContext.drawImage(myImage,0,0); // Draws the image on canvas\n          let imgData = myCanvas.toDataURL(\"image/jpeg\",0.75); // Assigns image base64 string in jpeg format to a variable\n\t\t  document.getElementById(\"layer1\").width = myCanvas.width;\n\t\t  document.getElementById(\"layer1\").height = myCanvas.height;\n\n        }\n      }\n    }\n  }\n  var can = document.getElementById(\"layer1\");\n\t\t\tvar ctx = can.getContext(\"2d\");\n\t\t\t\n  \t\tfunction imageMove(e)\n\t\t{\n\t\t\tconsole.log(e.clientX, e.clientY);\n\t\t\tvar rect = document.getElementById(\"layer1\").getBoundingClientRect()\n\t\t\tvar x = e.clientX - rect.left;\n\t\t\tvar y = e.clientY - rect.top;\n\t\t\t//ctx.beginPath();\n\t\t\tvar boxSize = document.getElementById(\"box_size\").value;\n\t\t\tctx.clearRect(0,0,can.width,can.height);\n\t\t\tctx.strokeStyle =  \"#FF0000\";\n\t\t\tctx.strokeRect(x,y,16*boxSize,16*boxSize);\n\t\t\t//ctx.stroke();\n\t\t\t\n\t\t}\n\n\n\n\t\tfunction imageClick(e)\n\t\t{\n\t\t\tvar rect = document.getElementById(\"layer1\").getBoundingClientRect()\n\t\t\tvar cx = e.clientX - rect.left;\n\t\t\tvar cy = e.clientY - rect.top;\n\t\t\tvar boxSize = document.getElementById(\"box_size\").value;\n\t\t\tctxI = document.getElementById(\"layer0\").getContext(\"2d\");\n\t\t\t//console.log(boxSize, cx + x* boxSize, cy + y * boxSize,boxSize * 16,boxSize * 16);\n\t\t\tvar dat = ctxI.getImageData(cx, cy,boxSize * 16,boxSize * 16).data;\n\t\t\tdocument.getElementById(\"image_data\").innerHTML = \"\";\n\t\t\tvar total =\"\"\n\t\t\tfor(var x = 0;x < 16;x++)\n\t\t\t{\n\t\t\t\tfor(var y = 0; y < 16;y++)\n\t\t\t\t{\n\t\t\t\t\t\n\t\t\t\t\tvar ravg = 0.0;\n\t\t\t\t\tvar gavg = 0.0;\n\t\t\t\t\tvar bavg = 0.0;\n\t\t\t\t\tfor(var i = 0;i < boxSize;i++)\n\t\t\t\t\t{\n\t\t\t\t\t\tfor(var j = 0;j < boxSize;j++)\n\t\t\t\t\t\t{\n\t\t\t\t\t\t\travg += dat[((x  + y * boxSize*16)  * boxSize + i + j * boxSize * 16)* 4];\n\t\t\t\t\t\t\tgavg += dat[((x  + y * boxSize*16)  * boxSize + i + j * boxSize * 16)* 4 + 1];\n\t\t\t\t\t\t\tbavg += dat[((x  + y * boxSize*16)  * boxSize + i + j * boxSize * 16)* 4 + 2];\n\t\t\t\t\t\t\t\n\t\t\t\t\t\t}\n\t\t\t\t\t}\n\t\t\t\t\travg/= 1/255*(boxSize * 16) * (boxSize * 16);\n\t\t\t\t\tgavg/= 1/255*(boxSize * 16) * (boxSize * 16);\n\t\t\t\t\tbavg/= 1/255*(boxSize * 16) * (boxSize * 16);\n\t\t\t\t\travg = Math.round(ravg);\n\t\t\t\t\tgavg= Math.round(gavg);\n\t\t\t\t\tbavg = Math.round(bavg);\n\t\t\t\t\tdcan = document.getElementById(\"dbg_can\");\n\t\t\t\t\tdctx = dcan.getContext(\"2d\");\n\t\t\t\t\tdctx.fillStyle = rgbToHex(ravg,gavg,bavg);\n\t\t\t\t\tdctx.fillRect(16*x,16*y,16,16);\n\t\t\t\t\tconsole.log(ravg,gavg,bavg);\n\n\t\t\t\t\ttotal+= rgbToHex(ravg,gavg,bavg).substring(1);\n\n\t\t\t\t\t//var node = document.createElement(\"INPUT\");\n\t\t\t\t\t\n\t\t\t\t\t//node.name = componentToHex(x + 16 * y);\n\t\t\t\t\t//node.value = rgbToHex(ravg,gavg,bavg).substring(1);\n\t\t\t\t\t//document.getElementById(\"image_data\").appendChild(node);\n\t\t\t\t}\n\t\t\t}\n\t\t\tvar node = document.createElement(\"INPUT\");\n\t\t\t\t\t\n\t\t\t\t\tnode.name = \"data\"\n\t\t\t\t\tnode.value = total;\n\t\t\t\t\tdocument.getElementById(\"image_data\").appendChild(node);\n\n\n\t\t\n\t\t}\n\t\tnavA = document.getElementsByClassName(\"nav-button\");\n\t\tfor(var i =  navA.length - 1;i >= 0;i--)\n\t\t{\n\t\t\tnavA[i].addEventListener(\"click\",navClick);\n\t\t\tnavA[i].click();\n\t\t}\n\t\tdocument.getElementById(\"color_palette_submit\").addEventListener(\"click\",submitForm);\n\t\tdocument.getElementById(\"animation_settings_submit\").addEventListener(\"click\",submitForm);\n\t\t/*document.getElementById(\"layer1\").addEventListener(\"mousemove\",imageMove);\n\t\tdocument.getElementById(\"layer1\").addEventListener(\"mousedown\",imageClick);\n\t\tnavA = document.getElementsByClassName(\"nav-button\");\n\t\tfor(var i =  navA.length - 1;i >= 0;i--)\n\t\t{\n\t\t\tnavA[i].addEventListener(\"click\",navClick);\n\t\t\tnavA[i].click();\n\t\t}\n\t\t//document.getElementById(\"color_possible\").addEventListener(\"change\",colorUpdate);\n\t\t//document.getElementById(\"image_upload\").addEventListener(\"change\",imageUpload)\n\t\t//var e = {};\n\t\t//e.srcElement = document.getElementById(\"color_possible\");\n\t\t//colorUpdate(e)*/\n\t</script>\n</body>\n</html>"
};
static const httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};
static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.core_id = 0;
    config.task_priority = tskIDLE_PRIORITY;
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &echo);

        #if CONFIG_EXAMPLE_BASIC_AUTH
        httpd_register_basic_auth(server);
        #endif
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}
static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static esp_err_t basic_auth_get_handler(httpd_req_t *req)
{
    char *buf = NULL;
    size_t buf_len = 0;
    basic_auth_info_t *basic_auth_info = req->user_ctx;

    buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
    if (buf_len > 1) {
        buf = calloc(1, buf_len);
        if (!buf) {
            ESP_LOGE(TAG, "No enough memory for basic authorization");
            return ESP_ERR_NO_MEM;
        }

        if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
        } else {
            ESP_LOGE(TAG, "No auth value received");
        }

        char *auth_credentials = http_auth_basic(basic_auth_info->username, basic_auth_info->password);
        if (!auth_credentials) {
            ESP_LOGE(TAG, "No enough memory for basic authorization credentials");
            free(buf);
            return ESP_ERR_NO_MEM;
        }

        if (strncmp(auth_credentials, buf, buf_len)) {
            ESP_LOGE(TAG, "Not authenticated");
            httpd_resp_set_status(req, HTTPD_401);
            httpd_resp_set_type(req, "application/json");
            httpd_resp_set_hdr(req, "Connection", "keep-alive");
            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Hello\"");
            httpd_resp_send(req, NULL, 0);
        } else {
            ESP_LOGI(TAG, "Authenticated!");
            char *basic_auth_resp = NULL;
            httpd_resp_set_status(req, HTTPD_200);
            httpd_resp_set_type(req, "application/json");
            httpd_resp_set_hdr(req, "Connection", "keep-alive");
            asprintf(&basic_auth_resp, "{\"authenticated\": true,\"user\": \"%s\"}", basic_auth_info->username);
            if (!basic_auth_resp) {
                ESP_LOGE(TAG, "No enough memory for basic authorization response");
                free(auth_credentials);
                free(buf);
                return ESP_ERR_NO_MEM;
            }
            httpd_resp_send(req, basic_auth_resp, strlen(basic_auth_resp));
            free(basic_auth_resp);
        }
        free(auth_credentials);
        free(buf);
    } else {
        ESP_LOGE(TAG, "No auth header received");
        httpd_resp_set_status(req, HTTPD_401);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_set_hdr(req, "Connection", "keep-alive");
        httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Hello\"");
        httpd_resp_send(req, NULL, 0);
    }

    return ESP_OK;
}


static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}


static void httpd_register_basic_auth(httpd_handle_t server)
{
    basic_auth_info_t *basic_auth_info = calloc(1, sizeof(basic_auth_info_t));
    if (basic_auth_info) {
        basic_auth_info->username = CONFIG_EXAMPLE_BASIC_AUTH_USERNAME;
        basic_auth_info->password = CONFIG_EXAMPLE_BASIC_AUTH_PASSWORD;

        basic_auth.user_ctx = basic_auth_info;
        httpd_register_uri_handler(server, &basic_auth);
    }
}

// An HTTP GET handler 
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    // Get header value string length and allocate memory for length + 1,
     /// extra byte for null termination 
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        // Copy null terminated value string into buffer //
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    //Read URL query string length and allocate memory for length + 1,
    //extra byte for null termination //
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            //Get value of expected key from query string //
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    //Set some custom headers 
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    // Send response with custom headers and body set as the
    //  string passed in user context//
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    // After sending the HTTP response the old HTTP request
     /// headers are lost. Check if HTTP request headers can be read now. *
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }    

    time_t cur = time(NULL);



    if(1)
    {
        printf("boot time was <20\n");
        //xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
        
    }
    else
    {
        printf("no boot time was >20\n");
    }
    //boot_last_time = (long int)cur;
    return ESP_OK;
}
void print_hex(const char *s, int r)
{
  for(int i = 0; i < r;i ++)
    printf("%02x", (unsigned int) *s++);
  printf("\n");
}
static esp_err_t echo_post_handler(httpd_req_t *req)
{
        int ret, remaining = req->content_len;

    while (remaining > 0) {
        // Read the data for the request 
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, BUF_SIZE * sizeof(char)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                //Retry receiving if timeout occurred //
                continue;
            }
            return ESP_FAIL;
        }

        // Send back the same data 
        httpd_resp_send_chunk(req, buf, BUF_SIZE);
        remaining -= ret;

        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        //ESP_LOGI(TAG, "%.*s", ret, buf);
        print_hex(buf,ret);
        ESP_LOGI(TAG, "====================================");
        //Setting type, length
        int next_index = 0;

            switch(byte_combine(buf[next_index],buf[next_index + 1]))
            {
                //Color palette matrix
                case 0:
                for(int i = 0;i < 16;i++)
                {
                    struct color_param temp;
                    settings_handle->matrix_color_palette[i].a = byte_combine(buf[next_index + i * 6 + 2 + 2 + 0], buf[next_index + i * 6 + 2 + 2 + 1])/1.0;
                    settings_handle->matrix_color_palette[i].b = byte_combine(buf[next_index + i * 6 + 2 + 2 + 2], buf[next_index + i * 6 + 2 + 2 + 3])/1.0;
                    settings_handle->matrix_color_palette[i].c = byte_combine(buf[next_index + i * 6 + 2 + 2 + 4], buf[next_index + i * 6 + 2 + 2 + 5])/1.0;
                    //settings_handle->matrix_color_palette[i] = temp;
                    printf("%f %f %f RGB\n",settings_handle->matrix_color_palette[i].a,settings_handle->matrix_color_palette[i].b,settings_handle->matrix_color_palette[i].c);
                }
                break;
                //Color palette strip
                case 1:
                for(int i = 0;i < 16;i++)
                {
                    struct color_param temp;
                    temp.a = byte_combine(buf[next_index + i * 6 + 2 + 2 + 0], buf[next_index + i * 6 + 2 + 2 + 1])/1.0;
                    temp.b = byte_combine(buf[next_index + i * 6 + 2 + 2 + 2], buf[next_index + i * 6 + 2 + 2 + 3])/1.0;
                    temp.c = byte_combine(buf[next_index + i * 6 + 2 + 2 + 4], buf[next_index + i * 6 + 2 + 2 + 5])/1.0;
                    settings_handle->strip_color_palette[i] = temp;
                }
                
                break;

                case 2:
                //0-254 settings as uint16, scale appropriately and to correct type
                //msgtype msglen drop_time matrix_hue_speed matrix_brightness sensitivity
                //   2     12     int 0-9      int 0-9         float 0-9       float 0-9
                settings_handle->drop_time = (int)(byte_combine(buf[next_index + 4], buf[next_index + 5])/28.0);
                settings_handle->matrix_hue_speed = (int)(byte_combine(buf[next_index + 6], buf[next_index + 7])/28.0);
                settings_handle->matrix_brightness = (byte_combine(buf[next_index + 8], buf[next_index + 9])/28.0);
                settings_handle->sensitivity = (byte_combine(buf[next_index + 10], buf[next_index + 11])/28.0);
                break;

                case 3:
                //0-254 settings as uint16, scale appropriately and to correct type
                //msgtype msglen strip_hue_speed strip_brightness 
                //   2     8      int 0-9            float 0-9     
                settings_handle->strip_hue_speed = (int)(byte_combine(buf[next_index + 4], buf[next_index + 5])/28.0);
                settings_handle->strip_brightness = (byte_combine(buf[next_index + 6], buf[next_index + 7])/28.0);  

                break;
                
            }
            next_index +=  byte_combine(buf[next_index + 2], buf[next_index + 3]) ;
            printf("%u NEXT IND\n", next_index);
            printf("%d RET \n", ret);
        
    }
    return ESP_OK;
}
void website_controller_init(struct device_settings* handle)
{
    static httpd_handle_t server = NULL;
    settings_handle = handle;
    buf = malloc(BUF_SIZE * sizeof(char));
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
         uint8_t mac[6];
     wifi_mode_t mode;
     //esp_wifi_get_mode(&mode);
     //esp_wifi_get_mac(mode, &mac[0]);
     esp_base_mac_addr_get(mac);
     printf("MAC ADDRESS %x %x %x %x %x %x \n",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
#ifdef CONFIG_EXAMPLE_CONNECT_ETHERNET
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_ETHERNET

    
    printf("STARTING HTTP SERVER\n");
    server = start_webserver();
    printf("IS THE SERVER ON THIS CORE?\n");
    printf("\n\n%c ,%d\n MAIN\n", 'A',xPortGetCoreID());
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;
    esp_netif_ip_info_t ipf;
    esp_netif_get_ip_info(NULL, &ipf);
    char ip_str[100] ;
    ip_str[99] = NULL;
    esp_ip4addr_ntoa(&(ipf.ip),ip_str, 100);
    printf(" IP ADD %s \n\n",ip_str);
}
