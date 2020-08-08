using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.Models
{
    public class ModelContext
    {
        private ModelContainer _container;
        private IDictionary<string, string> _includeModels;


        /// <summary>
        /// Default construction.
        /// </summary>
        public ModelContext()
        {
            _includeModels = new Dictionary<string, string>();
        }


        /// <summary>
        /// Initialize.
        /// </summary>
        /// <param name="container"></param>
        public ModelContext(ModelContainer container)
        {
            _container = container;
            _includeModels = new Dictionary<string, string>();
            Init();
        }


        /// <summary>
        /// Get / Set the container.
        /// </summary>
        public ModelContainer AllModels
        {
            get { return _container; }
            set { _container = value; Init(); }
        }


        /// <summary>
        /// Names of models to be included in the processing.
        /// </summary>
        public IDictionary<string, string> IncludeModels
        {
            get { return _includeModels; }
        }



        /// <summary>
        /// Initialize by storing all the model chain inheritance paths for each model.
        /// </summary>
        public void Init()
        {
            AllModels.Init();
        }


        public bool CanProcess(Model model, Func<Model, bool> predicate2)
        {
            bool pass = predicate2(model);
            if (pass && IncludeModels.Count == 0)
                return true;

            if (pass && IncludeModels.ContainsKey(model.Name))
                return true;

            return false;
        }
    }
}
