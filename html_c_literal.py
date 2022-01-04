html = open("in.html").read()
css = open("in.css").read()
js = open("in.js").read()

def escape(text):
    return text.replace("\\","\\\\").replace("\n","\\n").replace("\"","\\\"").replace("\'","\\\'").replace("\t","\\t")

html = escape(html.replace("<!DOCTYPE html>", ""))
css = escape(css)
js = escape(js)
out = open("out.html","w")

out.write("<!DOCTYPE html>" + "<style>" +css+ "</style>\\n"+ "<script>" + js + "</script>\\n" + html)