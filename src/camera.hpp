#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "lt_math.hpp"

struct Key;

struct Frustum
{
    Vec3f  position;
    f32    ratio;
    f32    fovy;
    f32    znear;
    f32    zfar;

    Mat4f  projection;
    // Calculated attributes
    Quatf  front;
    Quatf  right;
    Quatf  up;
};

struct Camera
{
    static constexpr f32 ZNEAR = 0.1f;
    static constexpr f32 ZFAR = 1000.0f;

    enum class Direction { Left, Right, Forwards, Backwards };
    enum class RotationAxis { Up, Down, Right, Left };

    Frustum frustum;
    Vec3f   up_world;
    f32     move_speed;
    f32     rotation_speed;

	Frustum previous_frustum;

	Vec3f curr_direction;     
	Vec3f curr_rotation_axis;     

    Camera(Vec3f position, Vec3f front_vec, Vec3f up_world,
           f32 fovy, f32 ratio, f32 move_speed, f32 rotation_speed);

	// void add_frame_rotation(RotationAxis axis);
	void update(Key *kb);

    Mat4f view_matrix(f64 lag_offset) const;

private:
	void add_frame_movement(Direction dir);
	void add_frame_rotation(RotationAxis axis);

	inline void reset()
	{
		curr_direction = Vec3f(0);
		curr_rotation_axis = Vec3f(0);
	}
	inline void move() { frustum.position += (curr_direction * move_speed); }
    void rotate();
	// void calculate_render_frustum(f64 frame_position);
	
};

#endif // CAMERA_HPP
