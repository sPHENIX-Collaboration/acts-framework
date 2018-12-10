
#ifndef LINEARIZEDTRACKFACTORY_H
#define LINEARIZEDTRACKFACTORY_H

#include "LinearizedTrack.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Utilities/Definitions.hpp"
/**
 * @class LinearizedTrackFactory
 * From ATHENA:
 * Linearizes the measurement equation (dependance of track
 * parameters on the vertex position and track mometum at vertex)
 * at the vicinity of the user-provided linearization point.
 *
 * The measurement equation is linearized in the following way: 
 *
 * q_k= A_k (x_k - x_0k) + B_k (p_k - p_0k) + c_k
 *
 * where q_k are the parameters at perigee nearest to the lin point, 
 * x_k is the position of the vertex, p_k the track momentum at the vertex,
 * and c_k is the constant term of expansion. A_k and B_k are matrices
 * of derivatives, denoted hereafter as "positionJacobian" and "momentumJacobian"
 * respectively.
 *
 */

class LinearizedTrackFactory
{
public:

    /// Default constructor
    LinearizedTrackFactory();

    /// Destructor
    ~LinearizedTrackFactory();

    LinearizedTrack* linearizeTrack(const Acts::BoundParameters* params,
                                    const Acts::Vector3D& linPoint) const;
};

#endif