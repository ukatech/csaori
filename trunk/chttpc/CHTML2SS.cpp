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

string CHTML2SS::_itoa(unsigned int num) {
	char tmptxt[32];
	sprintf(tmptxt,"%d",num);
	return *(new string(tmptxt));
}

wstring CHTML2SS::translate(wstring& in, string& url) {

#ifdef TRANSLATE_DEBUG
wprintf(L"CHTML2SS::translate in=%s\n",in.c_str());
#endif

	string utf8in = SAORI_FUNC::UnicodeToMultiByte(in, CP_UTF8);
	string out = "";

#ifdef TRANSLATE_DEBUG
printf("CHTML2SS::translate UnicodeToMultiByte, utf8in=%s\n",utf8in.c_str());
printf("CHTML2SS::translate parseTree\n");
#endif

	Curl *cu = new Curl(url);

	HTML::ParserDom parser;
	tree<HTML::Node> dom = parser.parseTree(utf8in);
  
	tree<HTML::Node>::pre_order_iterator it = dom.begin();
	tree<HTML::Node>::pre_order_iterator end = dom.end();
	tree<HTML::Node>::pre_order_iterator pnode, gpnode;
	tree<HTML::Node>::sibling_iterator prnode;

	
	string tmp, tmp2;
	unsigned int liCount = 1;

#ifdef TRANSLATE_DEBUG
printf("CHTML2SS::node loop\n");
#endif
	for (; it != end; ++it) {
		if ((!it->isComment())) {
			if(it->isTag()) {

				prnode = dom.previous_sibling(it);
				while(prnode != NULL && !prnode->isTag() && (prnode = dom.previous_sibling(prnode)) != NULL && !prnode->isTag()) {} // Get pervious valid sibling

#ifdef TRANSLATE_DEBUG
				printf("CHTML2SS::it = %s\n", it->tagName().c_str());
				if(prnode != NULL) printf("CHTML2SS::prnode = %s\n", prnode->tagName().c_str());
#endif
				if(prnode != NULL && prnode->tagName().size() > 0 && prnode->tagName().size() < 100) {
					tmp = prnode->tagName();
					if(tmp == "tr" || tmp == "table" || tmp == "ol" || tmp == "ul") {
						out.append("\\n");
					}
					if(tmp == "ol") {
						liCount = 1;
					}

				}

#ifdef TRANSLATE_DEBUG
printf("CHTML2SS::node loop - parseAttributes\n");
#endif
				it->parseAttributes();
				tmp = it->tagName();
				if(tmp == "base") {
					delete cu;
					cu = new Curl(it->attribute("href").second);
				} else if(tmp == "img") {
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
#ifdef TRANSLATE_DEBUG
printf("CHTML2SS::node loop - text node\n");
#endif
				pnode = dom.parent(it);
				pnode->parseAttributes();
				tmp = pnode->tagName();

				if(tmp == "style" || tmp == "script") {
				} else if(tmp == "a") {
					string href = pnode->attribute("href").second;
					href = replaceAll(href.substr(0, href.find("#")),"&amp;","&");
					if(href.empty() || href.find("javascript:") == 0 ) {
						out.append(stripHTMLTags(it->text()));
					} else {
						if(href.find("//") == 0) { // href="//domain.com/files/file.htm"
							href= cu->scheme + ":" + href;
						} else if(href.find("/") == 0) { // href="/files/file.htm"
							href= cu->scheme + "://" + cu->domain + cu->path + href;
						} else if(href.find(":") != string::npos) {
						} else { // href="files/file.htm"
							href= cu->scheme + "://" + cu->domain + cu->path + href;
						}
						out.append("\\_a[" + href + "]" + stripHTMLTags(it->text()) + "\\_a");
					}
				} else if(tmp == "li") {
					gpnode = dom.parent(pnode);
					if(gpnode != NULL) {
						gpnode->parseAttributes();
						tmp2 = gpnode->tagName();

						if(tmp2 == "ol") {
							out.append(_itoa(liCount++) + ". " + stripHTMLTags(it->text()) + "\\n");
						} else {
							out.append("- " + stripHTMLTags(it->text()) + "\\n");
						}
					} else {
						out.append("- " + stripHTMLTags(it->text()) + "\\n");
					}
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
	out = replaceAll(out, "  ", " ");
	out = replaceAll(out, "\\n ", "\\n");
	out = replaceAll(out, "\\n\\n\\n", "\\n");
	wstring wout = SAORI_FUNC::MultiByteToUnicode(out, CP_UTF8);
	
	return wout;
}