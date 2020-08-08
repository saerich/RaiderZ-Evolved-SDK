/*
 * Author: Kishore Reddy
 * Url: http://commonlibrarynet.codeplex.com/
 * Title: CommonLibrary.NET
 * Copyright: � 2009 Kishore Reddy
 * License: LGPL License
 * LicenseUrl: http://commonlibrarynet.codeplex.com/license
 * Description: A C# based .NET 3.5 Open-Source collection of reusable components.
 * Usage: Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using System.Collections.Generic;
using System.Text;

using ComLib.ValidationSupport;


namespace ComLib.Entities
{

    /// <summary>
    /// Validator a domain object.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class EntityValidator : Validator, IEntityValidator 
    {
        public static readonly IEntityValidator Empty = new EntityValidator();


        public EntityValidator() : base()
        {
        }


        public EntityValidator(Func<ValidationEvent, bool> validator)
            : base(validator)
        {
        }


        /// <summary>
        /// Validate using the object and the entityaction.
        /// </summary>
        /// <param name="target">object to validate.</param>
        /// <param name="results">results to add validation errors to.</param>
        /// <param name="action">entity action being done.</param>
        /// <returns></returns>
        public virtual bool Validate(object target, IValidationResults results, EntityAction action)
        {
            return Validate(new ValidationEvent(target, results, action));
        }


        /// <summary>
        /// Get the entity action from the validation event's context.
        /// </summary>
        /// <param name="validationEvent"></param>
        /// <returns></returns>
        public EntityAction GetEntityAction(ValidationEvent validationEvent)
        {
            EntityAction action = EntityAction.Create;
            if (validationEvent.Context == null)
            {
                IEntity entity = (IEntity)validationEvent.Target;
                if (entity.IsPersistant())
                    action = EntityAction.Update;
            }
            else if ( validationEvent.Context is EntityAction)
            {
                action = (EntityAction)validationEvent.Context;
            }
            return action;
        }
    }
}
