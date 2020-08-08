using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.Queue
{
    public class Queues
    {
        /// <summary>
        /// Named queues.
        /// </summary>
        private static IDictionary<string, IQueueProcessor> _queues = new Dictionary<string, IQueueProcessor>();


        /// <summary>
        /// Add a new named queue processor w/ the specified name.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="handler"></param>
        public static void AddProcessorFor<T>(Action<IList<T>> handler)
        {
            string namedHandler = typeof(T).FullName;
            AddProcessorFor<T>(namedHandler, handler, 10);
        }


        /// <summary>
        /// Add a new named queue processor w/ the specified name.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="handler"></param>
        public static void AddProcessorFor<T>(int itemsToDequeuePerProcess, Action<IList<T>> handler)
        {
            string namedHandler = typeof(T).FullName;
            AddProcessorFor<T>(namedHandler, handler, itemsToDequeuePerProcess);
        }


        /// <summary>
        /// Add a new named queue processor w/ the specified name.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="namedHandler"></param>
        /// <param name="handler"></param>
        public static void AddProcessorFor<T>(string namedHandler, Action<IList<T>> handler)
        {
            AddProcessorFor<T>(namedHandler, handler, 10);
        }


        /// <summary>
        /// Add a new named queue processor w/ the specified name.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="namedHandler"></param>
        /// <param name="handler"></param>
        public static void AddProcessorFor<T>(string namedHandler, Action<IList<T>> handler, int itemsToDequeue)
        {
            IQueueProcessor processer = new QueueProcessor<T>(itemsToDequeue, handler);
            _queues[namedHandler] = processer;
        }


        /// <summary>
        /// Add a new named queue processor w/ the specified name.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="handler"></param>
        public static void AddProcessor<T>(IQueueProcessor processor)
        {
            _queues[typeof(T).FullName] = processor;
        }


        /// <summary>
        /// Add a new named queue processor w/ the specified name.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="handler"></param>
        public static void AddProcessor(string name, IQueueProcessor processor)
        {
            _queues[name] = processor;
        }



        /// <summary>
        /// Whether or not there is a named handler for the specified type.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static bool ContainsProcessorFor<T>()
        {
            string name = typeof(T).FullName;
            return _queues.ContainsKey(name);
        }


        /// <summary>
        /// Enqueue the item.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="item"></param>
        public static void Enqueue<T>(T item)
        {            
            Enqueue<T>(typeof(T).FullName, item);
        }


        /// <summary>
        /// Enqueue the item.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="namedProcesser"></param>
        /// <param name="item"></param>
        public static void Enqueue<T>(string namedProcesser, T item)
        {
            AssertHandlerFor(namedProcesser);

            var processer = _queues[namedProcesser] as IQueueProcessor<T>;
            processer.Enqueue(item);
        }


        /// <summary>
        /// Enqueue the list of items.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="items"></param>
        public static void Enqueue<T>(IList<T> items)
        {
            Enqueue<T>(typeof(T).FullName, items);
        }


        /// <summary>
        /// Enqueue the list of items.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="namedProcesser"></param>
        /// <param name="items"></param>
        public static void Enqueue<T>(string namedProcesser, IList<T> items)
        {
            AssertHandlerFor(namedProcesser);

            var processer = _queues[namedProcesser] as IQueueProcessor<T>;
            foreach (var item in items)
                processer.Enqueue(item);
        }



        /// <summary>
        /// Process the queue handler for the specified type.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        public static void Process<T>()
        {
            Process(typeof(T).FullName);
        }


        /// <summary>
        /// Process the queue handler associated w/ the specified name.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="namedProcesser"></param>
        public static void Process(string namedProcesser)
        {
            AssertHandlerFor(namedProcesser);

            var processor = _queues[namedProcesser] as IQueueProcessor;
            processor.Process();
        }


        /// <summary>
        /// Whether or not the queue hanlder for the specified type is busy.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static bool IsBusy<T>()
        {
            return IsBusy(typeof(T).FullName);
        }


        /// <summary>
        /// Whether or not the queue hanlder for the specified type is busy.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static bool IsBusy(string namedProcesser)
        {
            AssertHandlerFor(namedProcesser);

            var processor = _queues[namedProcesser] as IQueueProcessor;
            return processor.IsBusy;
        }


        /// <summary>
        /// Whether or not the queue hanlder for the specified type is IsIdle.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static bool IsIdle<T>()
        {
            return IsIdle(typeof(T).FullName);
        }


        /// <summary>
        /// Whether or not the queue hanlder for the specified type is IsIdle.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static bool IsIdle(string namedProcesser)
        {
            AssertHandlerFor(namedProcesser);

            var processor = _queues[namedProcesser] as IQueueProcessor;
            return processor.IsIdle;
        }


        /// <summary>
        /// Get the queue processor for the specified type.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static IQueueProcessor<T> GetQueue<T>()
        {
            return GetQueue(typeof(T).FullName) as IQueueProcessor<T>;
        }


        /// <summary>
        /// Get the metainfo for all the queues.
        /// </summary>
        /// <returns></returns>
        public static List<QueueStatus> GetMetaInfo()
        {
            List<QueueStatus> states = new List<QueueStatus>();
            foreach (var processorEntry in _queues)
            {
                var state = processorEntry.Value.GetStatus();
                state.Name = processorEntry.Key;
                states.Add(state);
            }
            return states;
        }


        /// <summary>
        /// Get queue processor w/ the specified name.
        /// </summary>
        /// <param name="namedProcesser"></param>
        /// <returns></returns>
        public static IQueueProcessor GetQueue(string namedProcesser)
        {
            AssertHandlerFor(namedProcesser);
            return _queues[namedProcesser] as IQueueProcessor;
        }



        private static void AssertHandlerFor(string namedHandler)
        {
            if (!_queues.ContainsKey(namedHandler))
                throw new ArgumentException("There is no named queue handler named : " + namedHandler);
        }
    }



    public class QueueStatus
    {
        /// <summary>
        /// The name of the queue.
        /// </summary>
        public string Name;


        /// <summary>
        /// The current state of the queue.
        /// </summary>
        public readonly QueueProcessState State;


        /// <summary>
        /// Number of items still in the queue.
        /// </summary>
        public readonly int Count;


        /// <summary>
        /// The last time the queue was processed.
        /// </summary>
        public readonly DateTime LastProcessDate;


        /// <summary>
        /// How many items are dequeued from this queue each time.
        /// </summary>
        public readonly int DequeueSize;


        /// <summary>
        /// Number of times the queue has been processed.
        /// </summary>
        public readonly int NumberOfTimesProcessed;


        /// <summary>
        /// Amount of time since the last process date.
        /// </summary>
        public readonly TimeSpan ElapsedTimeSinceLastProcessDate;


        /// <summary>
        /// Total number of times that have been processed.
        /// </summary>
        public readonly int TotalProcessed;


        /// <summary>
        /// Initializes a new instance of the <see cref="QueueStatus"/> class.
        /// </summary>
        /// <param name="state">The state.</param>
        /// <param name="count">The count.</param>
        /// <param name="lastProcessDate">The last process date.</param>
        /// <param name="dequeueSize">Size of the dequeue.</param>
        public QueueStatus(QueueProcessState state, int countItemsRemaining, DateTime lastProcessDate, int numberOfTimesProcessed, TimeSpan elaspedTime, int totalProcessed, int dequeueSize)
        {
            State = state;
            Count = countItemsRemaining;
            NumberOfTimesProcessed = numberOfTimesProcessed;
            LastProcessDate = lastProcessDate;
            DequeueSize = dequeueSize;
            ElapsedTimeSinceLastProcessDate = elaspedTime;
            TotalProcessed = totalProcessed;
        }
    }
}
