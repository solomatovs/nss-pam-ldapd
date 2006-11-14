/*
   shadow.c - service entry lookup routines
   This file was part of the nss-ldap library (as ldap-spwd.c) which
   has been forked into the nss-ldapd library.

   Copyright (C) 1997-2005 Luke Howard
   Copyright (C) 2006 West Consulting
   Copyright (C) 2006 Arthur de Jong

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
   MA 02110-1301 USA
*/

#include "config.h"

#ifdef HAVE_SHADOW_H

#include <stdlib.h>
#include <string.h>
#include <shadow.h>
#include <errno.h>
#ifdef HAVE_PROT_H
#define _PROT_INCLUDED
#endif
#ifdef HAVE_LBER_H
#include <lber.h>
#endif
#ifdef HAVE_LDAP_H
#include <ldap.h>
#endif
#if defined(HAVE_THREAD_H)
#include <thread.h>
#elif defined(HAVE_PTHREAD_H)
#include <pthread.h>
#endif

#include "ldap-nss.h"

static struct ent_context *sp_context = NULL;

static enum nss_status _nss_ldap_parse_sp(LDAPMessage *e,
                    struct ldap_state *pvt,
                    void *result,char *buffer,size_t buflen)
{
  struct spwd *sp = (struct spwd *) result;
  enum nss_status stat;
  char *tmp = NULL;

  stat =
    _nss_ldap_assign_userpassword (e, ATM (LM_SHADOW, userPassword),
                                   &sp->sp_pwdp, &buffer, &buflen);
  if (stat != NSS_STATUS_SUCCESS)
    return stat;

  stat =
    _nss_ldap_assign_attrval (e, ATM (LM_SHADOW, uid), &sp->sp_namp, &buffer,
                              &buflen);
  if (stat != NSS_STATUS_SUCCESS)
    return stat;

  stat =
    _nss_ldap_assign_attrval (e, AT (shadowLastChange), &tmp, &buffer,
                              &buflen);
  sp->sp_lstchg = (stat == NSS_STATUS_SUCCESS) ? _nss_ldap_shadow_date (tmp) : -1;

  stat =
    _nss_ldap_assign_attrval (e, AT (shadowMax), &tmp, &buffer, &buflen);
  sp->sp_max = (stat == NSS_STATUS_SUCCESS) ? atol (tmp) : -1;

  stat =
    _nss_ldap_assign_attrval (e, AT (shadowMin), &tmp, &buffer, &buflen);
  sp->sp_min = (stat == NSS_STATUS_SUCCESS) ? atol (tmp) : -1;

  stat =
    _nss_ldap_assign_attrval (e, AT (shadowWarning), &tmp, &buffer,
                              &buflen);
  sp->sp_warn = (stat == NSS_STATUS_SUCCESS) ? atol (tmp) : -1;

  stat =
    _nss_ldap_assign_attrval (e, AT (shadowInactive), &tmp, &buffer,
                              &buflen);
  sp->sp_inact = (stat == NSS_STATUS_SUCCESS) ? atol (tmp) : -1;

  stat =
    _nss_ldap_assign_attrval (e, AT (shadowExpire), &tmp, &buffer,
                              &buflen);
  sp->sp_expire = (stat == NSS_STATUS_SUCCESS) ? _nss_ldap_shadow_date (tmp) : -1;

  stat =
    _nss_ldap_assign_attrval (e, AT (shadowFlag), &tmp, &buffer, &buflen);
  sp->sp_flag = (stat == NSS_STATUS_SUCCESS) ? atol (tmp) : 0;

  _nss_ldap_shadow_handle_flag(sp);

  return NSS_STATUS_SUCCESS;
}

enum nss_status _nss_ldap_getspnam_r(const char *name,
                      struct spwd *result,
                      char *buffer,size_t buflen,int *errnop)
{
  LOOKUP_NAME(name, result, buffer, buflen, errnop, _nss_ldap_filt_getspnam,
              LM_SHADOW, _nss_ldap_parse_sp, LDAP_NSS_BUFLEN_DEFAULT);
}

enum nss_status _nss_ldap_setspent(void)
{
  LOOKUP_SETENT(sp_context);
}

enum nss_status _nss_ldap_getspent_r(struct spwd *result,
                      char *buffer,size_t buflen,int *errnop)
{
  LOOKUP_GETENT(sp_context, result, buffer, buflen, errnop,
                _nss_ldap_filt_getspent, LM_SHADOW, _nss_ldap_parse_sp,
                LDAP_NSS_BUFLEN_DEFAULT);
}

enum nss_status _nss_ldap_endspent(void)
{
  LOOKUP_ENDENT(sp_context);
}

#endif /* HAVE_SHADOW_H */