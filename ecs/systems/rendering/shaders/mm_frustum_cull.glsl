#[compute]
#version 450
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(push_constant) uniform PushConstants {
    vec4 planes[6];
} camera_frustum;

layout(set = 0, binding = 0, std430) restrict buffer MultiMeshData {
    mat2x3 aabbs[];        // aabb[0] = pos, aabb[1] = size
    mat4 transforms[];
    uint culled_objects[];
    int num_instances;
} multimesh_data;


// Helpers
vec3 extractPosition(mat4 m) {
    return vec3(m[3]);
}

vec3 extractScale(mat4 m) {
    return vec3(
        length(vec3(m[0])),
        length(vec3(m[1])),
        length(vec3(m[2]))
    );
}

mat2x3 transformAABB(mat2x3 aabb, mat4 m) {
    vec3 pos   = extractPosition(m);
    vec3 scale = extractScale(m);

    vec3 localPos  = aabb[0];
    vec3 localSize = aabb[1];

    vec3 localMin = localPos - 0.5 * localSize;
    vec3 localMax = localPos + 0.5 * localSize;

    // Apply scaling
    vec3 newMin = localMin * scale;
    vec3 newMax = localMax * scale;

    // Ensure ordering and apply translation
    vec3 finalMin = min(newMin, newMax) + pos;
    vec3 finalMax = max(newMin, newMax) + pos;

    return mat2x3(finalMin, finalMax);
}


bool is_aabb_outside_plane(vec4 plane, mat2x3 aabb) {
    // use the most positive vertex
    vec3 minP = aabb[0];
    vec3 maxP = aabb[1];
    vec3 positive = minP;

    if (plane.x > 0.0) positive.x = maxP.x;
    if (plane.y > 0.0) positive.y = maxP.y;
    if (plane.z > 0.0) positive.z = maxP.z;

    float d = dot(plane.xyz, positive) - plane.w;
    return d < 0.0;
}


// Kernel
void main() {
    uint gid = gl_GlobalInvocationID.x;
    if (gid >= uint(multimesh_data.num_instances)) return;
    int index = int(gid);

    mat2x3 world_aabb = transformAABB(multimesh_data.aabbs[index], multimesh_data.transforms[index]);

    bool culled = false;
    for (int i = 0; i < 6; ++i) {
        vec4 plane = u_frustum.planes[i]; // use the declared name
        if (is_aabb_outside_plane(plane, world_aabb)) {
            culled = true;
            break;
        }
    }

    multimesh_data.culled_objects[index] = culled ? 1u : 0u; // store as uint
}
