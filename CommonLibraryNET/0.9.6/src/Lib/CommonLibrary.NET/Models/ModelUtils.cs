using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.Common;


namespace ComLib.Models
{
    public class ModelUtils
    {
        /// <summary>
        /// Get the inheritance path of a model as list of models.
        /// </summary>
        /// <param name="ctx"></param>
        /// <param name="modelName"></param>
        /// <returns></returns>
        public static List<Model> GetModelInheritancePath(ModelContainer container, string modelName, bool sortOnProperties)
        {
            List<Model> chain = GetModelInheritancePath(container, modelName);
            if (sortOnProperties)
                Sort(chain);

            return chain;
        }


        /// <summary>
        /// Get the inheritance path of a model as list of models.
        /// </summary>
        /// <param name="ctx"></param>
        /// <param name="modelName"></param>
        /// <returns></returns>
        public static List<Model> GetModelInheritancePath(ModelContainer container, string modelName)
        {
            Model currentModel = container.ModelMap[modelName];
            string inheritancePath = ConvertNestedToFlatInheritance(currentModel, container);
            
            // No parents?
            if( inheritancePath.IndexOf(",") < 0 )
            {
                return new List<Model>() { container.ModelMap[inheritancePath] };
            }

            // Delimited.
            List<Model> modelChain = new List<Model>();
            string[] parents = inheritancePath.Split(',');
            foreach (string parent in parents)
            {
                Model model = container.ModelMap[parent];
                modelChain.Add(model);
            }
            return modelChain;
        }


        /// <summary>
        /// Traverses the nodes inheritance path to build a single flat delimeted line of 
        /// inheritance paths.
        /// e.g. returns "Job,Post,EntityBase"
        /// </summary>
        /// <returns></returns>
        public static string ConvertNestedToFlatInheritance(Model model, ModelContainer container)
        {
            // Return name of environment provided if it doesn't have 
            // any inheritance chain.
            if (string.IsNullOrEmpty(model.Inherits))
                return model.Name;

            // Single parent.
            if (model.Inherits.IndexOf(",") < 0)
            {
                // Get the parent.
                Model parent = container.ModelMap[model.Inherits.Trim()];
                return model.Name + "," + ConvertNestedToFlatInheritance(parent, container);
            }

            // Multiple parents.
            string[] parents = model.Inherits.Split(',');
            string path = model.Name;
            foreach (string parent in parents)
            {
                Model parentModel = container.ModelMap[model.Inherits.Trim()];
                path += "," + ConvertNestedToFlatInheritance(parentModel, container);
            }
            return path;
        }


        /// <summary>
        /// Sort the 
        /// </summary>
        /// <param name="modelChain"></param>
        public static void Sort(List<Model> modelChain)
        {
            modelChain.Sort(delegate(Model m1, Model m2) 
            {
                if (m1.PropertiesSortOrder > m2.PropertiesSortOrder)
                    return 1;
                if (m1.PropertiesSortOrder < m2.PropertiesSortOrder)
                    return -1;
                return 0;
            }
            );
        }
    }
}
