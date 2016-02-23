
/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _TMM_DTW_H_
#define _TMM_DTW_H_

#define BASE_COUNT			10
#define COMPARE_COUNT		10
#define K_COUNT				10
#define SIMILAR_INFINITY	9999999 // @@@@@@@@@@ TEMPORARILY, set as a number bigger than distance max value @@@@@@@@@@

class tmm_dtw {
public:
	tmm_dtw();
	~tmm_dtw();

	int template_matching(int *S, int *T, float table[][K_COUNT]); // returns similarity

private:
	int m_dtw[BASE_COUNT + 1][COMPARE_COUNT + 1];

	int min3(int a, int b, int c);
};

#endif /* _TMM_DTW_H_ */

