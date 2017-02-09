/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /CLAIMS/logical_operator/project.h
 *
 *  Created on: Sep 22, 2015
 *      Author: casa, ChenLingyun
 *       Email: geekchenlingyun@outlook.com
 *
 * Description:
 * it's a function which takes an input (e.g. a database row) and
 * produces an output (e.g. one of the columns from the row, or
 * perhaps some calculation based on multiple columns).
 */

#ifndef LOGICAL_OPERATOR_LOGICAL_PROJECT_H_
#define LOGICAL_OPERATOR_LOGICAL_PROJECT_H_
#include <string>
#include <vector>
#include <map>

#include "../common/expression/expr_node.h"
#include "../common/Mapping.h"
#include "../common/TypePromotionMap.h"
#include "../common/TypeCast.h"
#include "../common/Expression/qnode.h"
#include "../catalog/catalog.h"
#include "../logical_operator/logical_operator.h"
#include "../logical_operator/Requirement.h"
#include "../physical_operator/physical_operator_base.h"
#include "../physical_operator/physical_project.h"
using claims::common::ExprNode;
namespace claims {
namespace logical_operator {

class LogicalProject : public LogicalOperator {
 public:
  LogicalProject(LogicalOperator* child, vector<QNode*> expression_tree);
  LogicalProject(LogicalOperator* child, vector<ExprNode*> expression_list);

  virtual ~LogicalProject();

  /**
   * @brief Method description : construct a PlanContext from child
   * @return : constructed plan_context
   */
  PlanContext GetPlanContext();

  /**
   * @brief Method description: generate a few physical operations according to
   * this logical operation
   * @param block_size : give the size of block in physical plan
   * @return the total physical plan include the physical plan generated by
   * child operation
   */
  PhysicalOperatorBase* GetPhysicalPlan(const unsigned& kBlockSize);

  // brief Method description:NOT USED NOW !!!
  virtual bool GetOptimalPhysicalPlan(
      Requirement requirement, PhysicalPlanDescriptor& physical_plan_descriptor,
      const unsigned& kBlockSize = 4096 * 1024) {}
  /**
   * @brief Method description:Print the schema
   * @param level:initialized to zero
   */
  void Print(int level = 0) const;
  void GetTxnInfo(QueryReq& request) const override {
      child_->GetTxnInfo(request);
    }
    void SetTxnInfo(const Query& query) override  {
      child_->SetTxnInfo(query);
    }

 private:
  /**
   * @brief Method description:construct a schema from
   * attribute list of plan context
   */
  Schema* GetOutputSchema();

 private:
  PlanContext* plan_context_;
  LogicalOperator* child_;
  std::vector<ExprNode*> expr_list_;
  std::vector<QNode*> expression_tree_;
};

}  // namespace logical_operator
}  // namespace claims

#endif  // LOGICAL_OPERATOR_LOGICAL_PROJECT_H_
