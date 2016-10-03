# -*- coding: utf-8 -*-

import threading
import urllib
import urllib2
import time
url = "http://219.228.147.162:8097"
Thread_quantity =  100
runs = 10000
def send_request(number,turns):
	#strnum = str(turns)+"-"+str(number)
	#f0 = open(strnum,'w+')
	#f = open ('/home/imdb/git/CLAIMS/sbin/claims-test/python/sql1.test','r')
	#sql_string = ""
	#while True:
	#	c = f.read(1)
	#	if not c : break
	#	if c == ';':
	#		sql_string += c
			#print str
			#req = urllib2.Request(url)
	#req = urllib2.Request(url)
	req = urllib2.Request(url + urllib.quote("/CLAIMS/"+sql_string))
	res_data = urllib2.urlopen(req)
	res = res_data.read()
			#print res
			#f0.write(res)
	#sql_string = ""
			
	#		continue
	#	sql_string += c
	#f0.close()
	#f.close() 




if __name__ == '__main__':
	f = open ('/home/imdb/git/CLAIMS/sbin/claims-test/python/sql1.test','r')
	sql_string = ""
	while True:
		c = f.read(1)
		if not c: break
		if c == ';':
			sql_string +=c
			continue
		sql_string += c
	print sql_string
	for j in range(runs):
		threads = []
		for i in range(Thread_quantity):
			threads.append(threading.Thread(target = send_request,args = (i,j,) ) )
		for t in threads:
			if j == 0 : t.setDaemon(True)
			t.start()
		for i in threads:
			t.join()
		if j%20 == 0 :print "%d turns is over" %j
		#time.sleep(1)
	print "all over"
	
	
	#req = urllib2.Request(url + urllib.quote("/CLAIMS:select * from trade where trade_no = 1;"))
	
	#res_data = urllib2.urlopen(req)
	#res = res_data.read()
	#print res

	
