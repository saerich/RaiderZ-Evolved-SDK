using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib
{
    public class EnumerableHelper
    {
        /// <summary>
        /// Calls the action by supplying the start and end index.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="itemCount"></param>
        /// <param name="cols"></param>
        /// <param name="action"></param>
        public static void ForEachByCols(int itemCount, int cols, Action<int, int> action)
        {
            if (itemCount == 0)
                return;

            if (itemCount <= cols)
            {
                action(0, itemCount - 1);
                return;
            }

            int startNdx = 0;
            while (startNdx < itemCount)
            {
                // 1. startNdx = 0 .. endNdx = 2
                // 2. startNdx = 3 .. endNdx = 5
                int endNdx = startNdx + (cols - 1);
                if (endNdx >= itemCount)
                    endNdx = itemCount - 1;

                action(startNdx, endNdx);
                startNdx = endNdx + 1;
            }
        }
    }
}
