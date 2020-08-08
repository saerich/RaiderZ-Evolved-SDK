using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using ComLib;
using ComLib.Models;


namespace ComLib.CodeGeneration
{
    public interface ICodeBuilder
    {
        BoolMessageItem<ModelContainer> Process(ModelContext ctx);
    }
}
