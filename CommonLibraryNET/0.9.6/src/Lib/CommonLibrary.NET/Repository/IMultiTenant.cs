using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.Entities
{
    public interface IMultiTenant
    {
        int TenantId { get; set; }
    }
}
