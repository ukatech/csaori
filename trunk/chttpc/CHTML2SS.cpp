// for SAORI_FUNC
#include "csaori.h"

// htmlcxx
#include "html/ParserDom.h"
#include "html/utils.h"

// striphtml
#include "striphtml.h"

#include "CHTML2SS.h"

#ifdef TRANSLATE_DEBUG
#include <stdio.h>
#endif

using namespace htmlcxx;
using namespace std;

wstring CHTML2SS::translate(std::wstring& in) {
#ifdef TRANSLATE_DEBUG
printf("CHTML2SS::translate UnicodeToMultiByte\n");
#endif
	std::string utf8in = SAORI_FUNC::UnicodeToMultiByte(in, CP_UTF8);
	std::string out = "";

#ifdef TRANSLATE_DEBUG
printf("CHTML2SS::translate parseTree\n");
#endif

	HTML::ParserDom parser;
	tree<htmlcxx::HTML::Node> dom = parser.parseTree(utf8in);
  
	tree<HTML::Node>::pre_order_iterator it = dom.begin();
	tree<HTML::Node>::pre_order_iterator end = dom.end();
	tree<HTML::Node>::pre_order_iterator pnode;
	tree<HTML::Node>::pre_order_iterator prnode;

	
	string tmp;
	for (; it != end; ++it) {
		if ((!it->isComment())) {
			if(it->isTag()) {

				prnode = dom.previous_sibling(it);
				if(prnode != NULL) {
					tmp = prnode->tagName();
					if(tmp == "tr" || tmp == "table") {
						out.append("\\n");
					}
				}

				it->parseAttributes();
				tmp = it->tagName();
				if(tmp == "img") {
					tmp = it->attribute("alt").second;
					if(tmp == "")
						tmp = it->attribute("src").second;
					out.append("(img:" + tmp + ")");
				} else if(tmp == "br" || tmp == "p") {
					out.append("\\n");
				} else if(tmp == "hr") {
					out.append("\\n-----------------------------------------------\\n");
				} else if(tmp == "input") {
					tmp = it->attribute("type").second;
					if(tmp == "button" || tmp == "submit" || tmp == "reset" || tmp == "text" || tmp == "") {
						out.append("[" + it->attribute("value").second + "]");
					} else if(tmp == "checkbox") {
						out.append(it->attribute("checked").first == 1 ? "\\[X\\]" : "\\[ \\]");
					} else if(tmp == "radio") {
						out.append(it->attribute("checked").first == 1 ? "(*)" : "( )");
					}
				}
			} else {
				pnode = dom.parent(it);
				pnode->parseAttributes();
				tmp = pnode->tagName();

				if(tmp == "style" || tmp == "script") {
				} else if(tmp == "a") {
					out.append("\\_a[" + pnode->attribute("href").second + "]" + stripHTMLTags(it->text()) + "\\_a");
				} else if(tmp == "li") {
					out.append(" - " + stripHTMLTags(it->text()) + "\\n");
				} else {
					out.append(stripHTMLTags(it->text()));
				}
			}
		}
	}
#ifdef TRANSLATE_DEBUG
printf("CHTML2SS::translate before return\n");
#endif

	out = replaceAll(out, "\r", "");
	out = replaceAll(out, "\n", "");
	out = replaceAll(out, "\\n\\n", "\\n");
	wstring wout = SAORI_FUNC::MultiByteToUnicode(out, CP_UTF8);
	
	return wout;
}