/*
    Copyright (C) 2014-2016 Islog

    This file is part of Leosac.

    Leosac is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Leosac is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "core/auth/AuthFwd.hpp"
#include "core/credentials/CredentialFwd.hpp"
#include "tools/ToolsFwd.hpp"
#include "tools/db/db_fwd.hpp"

namespace Leosac
{
/**
 * A SecurityContext is used to query permission while doing
 * an operation.
 *
 * For example, JSON serializers could use a SecurityContext to determine what
 * to serialize.
 *
 * todo: fix doc
 */
class SecurityContext
{
  public:
    // Forward declare the union so we can write cast operator.
    union ActionParam;
    enum class Action
    {
        USER_CREATE,
        USER_READ,
        USER_READ_EMAIL,
        USER_UPDATE,
        USER_CHANGE_PASSWORD,

        /**
         * Editing rank means being able to become administrator.
         */
        USER_UPDATE_RANK,
        /**
         * Can we enable/disable the user or change its
         * validity period ?
         */
        USER_MANAGE_VALIDITY,
        USER_DELETE,

        GROUP_CREATE,
        GROUP_READ,
        GROUP_UPDATE,
        GROUP_DELETE,
        GROUP_SEARCH,

        /**
         * Ability to list member of a group.
         * The ability to list membership gives USER_READ access against
         * the user whose membership is listed.
         */
        GROUP_LIST_MEMBERSHIP,
        GROUP_MEMBERSHIP_JOINED,
        GROUP_MEMBERSHIP_LEFT,
        MEMBERSHIP_READ,

        CREDENTIAL_READ,
        CREDENTIAL_UPDATE,
        CREDENTIAL_CREATE,
        CREDENTIAL_DELETE,

        SCHEDULE_READ,
        SCHEDULE_UPDATE,
        SCHEDULE_CREATE,
        SCHEDULE_DELETE,

        DOOR_READ,
        DOOR_UPDATE,
        DOOR_CREATE,
        DOOR_DELETE,
        DOOR_SEARCH,

        /**
         * Retrieve SMTP configuration
         */
        SMTP_GETCONFIG,

        LOG_READ,
    };

    struct GroupActionParam
    {
        Auth::GroupId group_id;

        operator ActionParam();
    };

    struct UserActionParam
    {
        Auth::UserId user_id;

        operator ActionParam();
    };

    struct CredentialActionParam
    {
        Cred::CredentialId credential_id;

        operator ActionParam();
    };

    struct MembershipActionParam
    {
        Auth::UserGroupMembershipId membership_id;
        Auth::GroupId group_id; // for create/delete
        Auth::UserId user_id;   // for create/delete
        Auth::GroupRank rank;   // for create

        operator ActionParam();
    };

    struct ScheduleActionParam
    {
        Tools::ScheduleId schedule_id;

        operator ActionParam();
    };

    struct DoorActionParam
    {
        Auth::DoorId door_id;

        operator ActionParam();
    };

    union ActionParam {
        GroupActionParam group;
        MembershipActionParam membership;
        UserActionParam user;
        CredentialActionParam cred;
        ScheduleActionParam sched;
        DoorActionParam door;
    };

    SecurityContext(DBServicePtr dbsrv);

    virtual bool check_permission(Action a, const ActionParam &ap) const;

  protected:
    DBServicePtr dbsrv_;
};

/**
 * A security context for system operation.
 *
 * Basically, this security context always grants all permissions.
 */
class SystemSecurityContext : public SecurityContext
{
  public:
    SystemSecurityContext(DBServicePtr dbsrv);

    static SecurityContext &instance();
    virtual bool check_permission(Action a, const ActionParam &ap) const override;
};
}
