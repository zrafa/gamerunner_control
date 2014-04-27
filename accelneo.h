/*
 * Copyright (C) 2008 by OpenMoko, Inc.
 * Written by Paul-Valentin Borza <gestures@borza.ro>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef ACCELNEO_H_
#define ACCELNEO_H_

#ifndef ACCEL_T
#define ACCEL_T
/* acceleration */
typedef struct accel_3d_t {
	float val[3];
} accel_3d_t;

#endif

#ifndef HANDLE_ACCEL_T
#define HANDLE_ACCEL_T
/* handle for acceleration */
typedef void (* handle_recv_3d_t)(unsigned char pressed, struct accel_3d_t accel);

#endif

typedef enum neo_accel {
	neo_accel2 = 2,
	neo_accel3 = 3
} neo_accel;

typedef struct neo_t {
	int accel_desc;
	int screen_desc;
	/* callback for acceleration */
	handle_recv_3d_t handle_recv;
} neo_t;

/* opens one accelerometer and the touchscreen */
unsigned char neo_open(struct neo_t *neo, enum neo_accel w_accel);
/* closes the accelerometer and the touchscreen */
void neo_close(struct neo_t *neo);
/* begins reading the accelerometer and the touchscreen */
void neo_begin_read(struct neo_t *neo);

#endif /*ACCELNEO_H_*/
