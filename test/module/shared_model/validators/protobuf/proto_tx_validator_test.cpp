/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "module/shared_model/validators/validators_fixture.hpp"
#include "validators/protobuf/proto_transaction_validator.hpp"

class ProtoTxValidatorTest : public ValidatorsTest {
 protected:
  iroha::protocol::Transaction generateEmptyTransaction() {
    std::string creator_account_id = "admin@test";

    TestTransactionBuilder builder;
    auto tx = builder.creatorAccountId(creator_account_id)
                  .createdTime(created_time)
                  .quorum(1)
                  .build()
                  .getTransport();
    return tx;
  }

  iroha::protocol::Transaction generateCreateRoleTransaction(
      const std::string &role_name,
      iroha::protocol::RolePermission permission) {
    auto tx = generateEmptyTransaction();

    iroha::protocol::CreateRole cr;
    cr.set_role_name(role_name);
    cr.add_permissions(permission);

    tx.mutable_payload()
        ->mutable_reduced_payload()
        ->add_commands()
        ->mutable_create_role()
        ->CopyFrom(cr);
    return tx;
  }
};

/**
 * @given iroha::protocol::Transaction with defined command
 * @when it is validated
 * @then answer with no errors is returned
 */
TEST_F(ProtoTxValidatorTest, CommandIsSet) {
  auto tx = generateEmptyTransaction();
  // add not set command
  iroha::protocol::CreateDomain cd;
  cd.set_domain_id(domain_id);
  cd.set_default_role(role_name);

  tx.mutable_payload()
      ->mutable_reduced_payload()
      ->add_commands()
      ->mutable_create_domain()
      ->CopyFrom(cd);
  shared_model::proto::Transaction proto_tx(tx);

  shared_model::validation::ProtoTransactionValidator<
      shared_model::validation::FieldValidator,
      shared_model::validation::CommandValidatorVisitor<
          shared_model::validation::FieldValidator>>
      validator;
  auto answer = validator.validate(proto_tx);
  ASSERT_FALSE(answer.hasErrors()) << answer.reason();
}

/**
 * @given iroha::protocol::Transaction with undefined command
 * @when it is validated
 * @then answer with errors is returned
 */
TEST_F(ProtoTxValidatorTest, CommandNotSet) {
  auto tx = generateEmptyTransaction();
  // add not set command
  tx.mutable_payload()->mutable_reduced_payload()->add_commands();
  shared_model::proto::Transaction proto_tx(tx);

  shared_model::validation::ProtoTransactionValidator<
      shared_model::validation::FieldValidator,
      shared_model::validation::CommandValidatorVisitor<
          shared_model::validation::FieldValidator>>
      validator;
  auto answer = validator.validate(proto_tx);
  ASSERT_TRUE(answer.hasErrors());
}

/**
 * @given iroha::protocol::Transaction containing create role transaction with
 * valid role permission
 * @when it is validated
 * @then answer with errors is returned
 */
TEST_F(ProtoTxValidatorTest, CreateRoleValid) {
  auto tx = generateCreateRoleTransaction(
      role_name, iroha::protocol::RolePermission::can_read_assets);

  shared_model::proto::Transaction proto_tx(tx);

  shared_model::validation::ProtoTransactionValidator<
      shared_model::validation::FieldValidator,
      shared_model::validation::CommandValidatorVisitor<
          shared_model::validation::FieldValidator>>
      validator;
  auto answer = validator.validate(proto_tx);
  ASSERT_FALSE(answer.hasErrors());
}

/**
 * @given iroha::protocol::Transaction containing create role transaction with
 * undefined role permission
 * @when it is validated
 * @then answer with no errors is returned
 */
TEST_F(ProtoTxValidatorTest, CreateRoleInvalid) {
  auto tx = generateCreateRoleTransaction(
      role_name, static_cast<iroha::protocol::RolePermission>(-1));

  shared_model::proto::Transaction proto_tx(tx);

  shared_model::validation::ProtoTransactionValidator<
      shared_model::validation::FieldValidator,
      shared_model::validation::CommandValidatorVisitor<
          shared_model::validation::FieldValidator>>
      validator;
  auto answer = validator.validate(proto_tx);
  ASSERT_TRUE(answer.hasErrors());
}
