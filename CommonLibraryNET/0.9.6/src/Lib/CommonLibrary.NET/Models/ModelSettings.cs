using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ComLib.Data;


namespace ComLib.Models
{
    public class ModelBuilderSettings
    {
        /// <summary>
        /// Database connectio used to create the tables associated with a model.
        /// </summary>
        public ConnectionInfo Connection { get; set; }


        /// <summary>
        /// Assembly name.
        /// </summary>
        public string AssemblyName { get; set; }


        /// <summary>
        /// Location of the generated code.
        /// </summary>
        public string ModelCodeLocation { get; set; }


        /// <summary>
        /// Location of the templates for code generation
        /// </summary>
        public string ModelCodeLocationTemplate { get; set; }


        /// <summary>
        /// Location where the sql schema files are created.
        /// </summary>
        public string ModelInstallLocation { get; set; }


        /// <summary>
        /// Location where the sql schema files are created.
        /// </summary>
        public string ModelCodeLocationUI { get; set; }


        /// <summary>
        /// Location where the UI templates are located.
        /// </summary>
        public string ModelCodeLocationUITemplate { get; set; }


        /// <summary>
        /// Location where orm mapping file should be created.
        /// </summary>
        public string ModelOrmLocation { get; set; }


        /// <summary>
        /// Location of the stored procedure templates.
        /// </summary>
        public string ModelDbStoredProcTemplates { get; set; }


        /// <summary>
        /// Location where orm mapping file should be created.
        /// </summary>
        public DbCreateType DbAction_Create { get; set; }
    }
}
