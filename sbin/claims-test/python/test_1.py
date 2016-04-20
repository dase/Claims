# -*- coding: utf-8 -*-

import urllib
import urllib2
url = "http://219.228.147.162:8097"


if __name__ == '__main__':
	f0 = open ('out','w+')
	f = open ('/home/imdb/git/CLAIMS/sbin/claims-test/python/sql1.test','r')
	#flag = false
	str = ""
	while True:
		c = f.read(1)
		if not c : break
		if c == ';':
			str += c
			#print str
			req = urllib2.Request(url + urllib.quote("/CLAIMS/"+str))
			res_data = urllib2.urlopen(req)
			res = res_data.read()
			print res
			f0.write(res)
			str = ""
			
			continue
		str += c
	f0.close()
	f.close()
	
	
	#req = urllib2.Request(url + urllib.quote("/CLAIMS:select * from trade where trade_no = 1;"))
	
	#res_data = urllib2.urlopen(req)
	#res = res_data.read()
	#print res

	
