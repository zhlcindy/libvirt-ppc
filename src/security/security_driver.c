/*
 * Copyright (C) 2008-2012 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     James Morris <jmorris@namei.org>
 *     Dan Walsh <dwalsh@redhat.com>
 *
 */
#include <config.h>
#include <string.h>

#include "virterror_internal.h"
#include "logging.h"

#include "security_driver.h"
#ifdef WITH_SECDRIVER_SELINUX
# include "security_selinux.h"
#endif

#ifdef WITH_SECDRIVER_APPARMOR
# include "security_apparmor.h"
#endif

#include "security_nop.h"

#define VIR_FROM_THIS VIR_FROM_SECURITY

static virSecurityDriverPtr security_drivers[] = {
#ifdef WITH_SECDRIVER_SELINUX
    &virSecurityDriverSELinux,
#endif
#ifdef WITH_SECDRIVER_APPARMOR
    &virAppArmorSecurityDriver,
#endif
    &virSecurityDriverNop, /* Must always be last, since it will always probe */
};

virSecurityDriverPtr virSecurityDriverLookup(const char *name,
                                             const char *virtDriver)
{
    virSecurityDriverPtr drv = NULL;
    int i;

    VIR_DEBUG("name=%s", NULLSTR(name));

    for (i = 0; i < ARRAY_CARDINALITY(security_drivers) && !drv ; i++) {
        virSecurityDriverPtr tmp = security_drivers[i];

        if (name &&
            STRNEQ(tmp->name, name))
            continue;

        switch (tmp->probe(virtDriver)) {
        case SECURITY_DRIVER_ENABLE:
            VIR_DEBUG("Probed name=%s", tmp->name);
            drv = tmp;
            break;

        case SECURITY_DRIVER_DISABLE:
            VIR_DEBUG("Not enabled name=%s", tmp->name);
            break;

        default:
            return NULL;
        }
    }

    if (!drv) {
        virReportError(VIR_ERR_INTERNAL_ERROR,
                       _("Security driver %s not found"),
                       NULLSTR(name));
        return NULL;
    }

    return drv;
}
