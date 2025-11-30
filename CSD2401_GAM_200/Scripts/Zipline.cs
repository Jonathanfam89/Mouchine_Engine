using System.Numerics;

namespace GameScripts
{
    /// <summary>
    /// A simple data class representing a Zipline in the game world.
    /// In the current engine architecture, the start and end points must be defined
    /// directly in world space, as there is no API to convert local-to-world coordinates.
    /// </summary>
    public class Zipline
    {
        /// <summary>
        /// The name of the GameObject this Zipline is associated with.
        /// </summary>
        public string GameObjectName;

        /// <summary>
        /// The starting point of the zipline in world coordinates.
        /// </summary>
        public Vector2 WorldStartPoint;

        /// <summary>
        /// The ending point of the zipline in world coordinates.
        /// </summary>
        public Vector2 WorldEndPoint;

        /// <summary>
        /// Default constructor.
        /// </summary>
        public Zipline()
        {
            GameObjectName = "";
            WorldStartPoint = Vector2.Zero;
            WorldEndPoint = Vector2.Zero;
        }

        /// <summary>
        /// Constructor to create a Zipline with defined points.
        /// </summary>
        /// <param name="name">The name of the associated GameObject.</param>
        /// <param name="start">The world-space start point.</param>
        /// <param name="end">The world-space end point.</param>
        public Zipline(string name, Vector2 start, Vector2 end)
        {
            GameObjectName = name;
            WorldStartPoint = start;
            WorldEndPoint = end;
        }
    }
}