/*
 * StringToJson.h
 *
 *  Created on: 26/1/2016
 *      Author: hcs
 */

#ifndef HTTPSERVER_RESULTMANAGE_HPP_
#define HTTPSERVER_RESULTMANAGE_HPP_

#include <string>
#include "../httpserver/json/json.h"
#include "../Client/ClientResponse.h"
#include "../Daemon/Daemon.h"
namespace httpserver{

static void result_manage(std::string &str, ExecutedResult &res){
	Json::Value root;
	Json::Value reply_item;
	Json::Value result_item;
	Json::Value schema_item;
	Json::Value data_item;
	Json::Value temp;
	vector<string> resutls_json;
	resutls_json.push_back("schema");
	resutls_json.push_back("data");
	ResultSet *rs = res.result_;
	vector<string> &col_name_list = rs->column_header_list_;
	DynamicBlockBuffer::Iterator it = rs->createIterator();
	if(res.status_){
		//result_ is not empty
		if(res.result_ != NULL){
			BlockStreamBase *block;
			for(int i = 0; i< rs->schema_->getncolumns(); i++){
				result_item[resutls_json[0]].append(col_name_list[i]);
			}
			cout<<rs->schema_->getncolumns()<<endl;
			while(block = it.nextBlock()){
				BlockStreamBase::BlockStreamTraverseIterator *block_it = block->createIterator();
				void *tuple;

				while(tuple = block_it->nextTuple()){
					for(int i = 0; i< rs->schema_->getncolumns(); i++){
						data_item.append(Json::Value(rs->schema_->getColumnValue(tuple,i)));
					}
					result_item[resutls_json[1]].append(data_item);
					data_item.clear();
				}
			}
			reply_item["status"] = "ok";
			reply_item["info"] = "";
			reply_item["error info"] = "";
			reply_item["warning info"] = "";
			reply_item["execute time"] = std::to_string(res.result_time_) + " sec";
			reply_item["result"]= result_item;
			//reply_item = result_item;
			root.append(reply_item);
		}
		else{
			if("" == res.warning_){
				//std::cout<<"tu putaopi zhiqian\n";
				reply_item["status"] = "ok";
				reply_item["info"] = res.info_;
				reply_item["error info"] = "";
				reply_item["warning info"] = "";
				reply_item["execute time"] = std::to_string(res.result_time_) + " sec";

				//std::cout<<"tu putaopi zhizhong\n";
				reply_item["result"]= "";
				//reply_item = result_item;
				root.append(reply_item);
				//std::cout<<"tu putaopi zhihou\n";

			}
			else{

				reply_item["status"] = "warning";
				reply_item["info"] = "";
				reply_item["error info"] = "";
				std::string info = res.info_ + "\n\nWARNINGS:\n" + res.warning_ + "\n";
				reply_item["warning info"] = info;
				reply_item["execute time"] = std::to_string(res.result_time_) + " sec";
				reply_item["result"]= "";
				//reply_item = result_item;
				root.append(reply_item);
				LOG(INFO)<< "to send change response-- status:" << CHANGEDD
				<< "  length:" << info.length()
				<< "  content:" << info.c_str()
				<< " warnings: " << res.warning_.c_str() << std::endl;
			}
			std::cout<<"resulttojson is catch the show tables;\n";
			std::cout<<res.info_<<std::endl;
		}
	}
	else{
		//ERROR
		if("" == res.warning_){
			reply_item["status"] = "error";
			reply_item["info"] = "";
			reply_item["error info"] = res.error_info_;
			reply_item["warning info"] = "";
			reply_item["execute time"] = std::to_string(res.result_time_) + " sec";
			reply_item["result"]= "";
			root.append(reply_item);
		}
		else{

			reply_item["status"] = "error&warning";
			reply_item["info"] = "";
			reply_item["error info"] = res.error_info_;
			std::string info = res.error_info_ + "\n\nWARNINGS:\n" + res.warning_ + "\n";
			reply_item["warning info"] = info;
			reply_item["execute time"] = std::to_string(res.result_time_) + " sec";
			reply_item["result"]= "";
			root.append(reply_item);
			ClientListenerLogging::log(
					"to send err response-- status:%d  length:%d  content:%s"
							"warnings: %s", ERROR, info.length(),
					info.c_str(), res.warning_.c_str());
		}
	}

	Json::FastWriter fastwriter;
	str = fastwriter.write(reply_item);
	str += '\n';
	//cout<< str <<endl;

}


}//httpserver

#endif /* HTTPSERVER_RESULTMANAGE_HPP_ */
