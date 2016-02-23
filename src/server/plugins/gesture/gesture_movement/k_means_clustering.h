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

#ifndef _K_MEANS_CLUSTERING_H_
#define _K_MEANS_CLUSTERING_H_

#define NCOUNT 10 // each gesture's accel data count : 10
#define MCOUNT 20 // all gesture count : 20
#define KCOUNT 10 // clusters : 10

typedef struct accel_t {
	float x;
	float y;
	float z;
} accel_t;

typedef struct mydata_t {
	accel_t accel;
	int ci; // cluster_index
	float ed; // euclid_distance
} mydata_t;

static accel_t pre_accel[MCOUNT][NCOUNT] = {
{ // 1 : 10
{-1.164108, 7.643872, 0.604188}, {7.069595, 8.834301, -0.259621}, {5.647062, 8.043473, -1.317249}, {-2.474179, 8.262416, 0.574277}, {-3.421736, 8.587841, 0.671187},
{5.581259, 7.938189, -0.061017}, {9.767502, 9.393025, 0.080160}, {1.715654, 8.282756, 0.666401}, {-1.267000, 8.651250, 0.949951}, {-0.745364, 9.410972, 0.936790},
},
{// 2 : 10
{-5.453243, 7.904689, 0.921237}, {6.577869, 7.850852, -0.715454}, {9.742377, 8.766105, -0.263210}, {1.098306, 7.858029, 0.154337}, {-3.128615, 8.367701, -0.601795},
{0.746561, 6.179464, -0.535992}, {8.614161, 8.823533, 0.047856}, {7.701300, 9.040084, -0.531207}, {-0.357727, 8.098508, 0.597009}, {-0.977468, 8.700303, 1.269392},
},
{ // 3 : 10
{-5.614758, 7.998010, 0.750150}, {8.346166, 7.079165, -1.014557}, {10.950752, 8.142776, -1.178465}, {3.942175, 7.050452, -0.910469}, {-3.151347, 7.777870, -0.336192},
{-5.108676, 5.765506, -0.597009}, {9.828518, 8.189436, -0.425922}, {9.214760, 8.612966, -0.235693}, {-1.032503, 7.598408, -0.379262}, {-1.619941, 8.426324, 0.667597},
},
{ // 4 : 10
{-3.512664, 9.358330, 0.881755}, {3.109473, 6.077770, -0.556331}, {12.928421, 7.389036, -1.883151}, {4.427918, 6.616154, -1.057628}, {-3.834498, 7.769495, -0.506082},
{-3.878765, 5.020142, 0.917647}, {11.827723, 7.232306, -1.493121}, {9.016156, 6.856634, -0.398405}, {-1.653440, 7.653443, -0.056231}, {-0.891326, 9.035298, 1.538585},
},
{ // 5 : 10
{-5.462814, 4.914857, 2.292324}, {11.674581, 7.300501, -4.130012}, {9.611969, 6.905686, -1.069592}, {-2.845066, 7.434499, 0.046660}, {-5.636294, 7.865208, 0.811167},
{4.771288, 5.461617, 0.264407}, {11.679367, 7.270591, -1.033699}, {2.652444, 7.524230, -0.836291}, {-1.777867, 8.413164, 0.460618}, {-0.637687, 9.296116, 1.083949},
},
{ // 6 : 10
{-4.633701, 8.845069, 1.204786}, {4.761717, 3.312863, 0.726222}, {13.666606, 5.358726, -4.336991}, {6.853044, 5.657829, -1.200001}, {-1.624727, 7.562516, -1.148555},
{-3.381058, 8.537591, -0.117248}, {0.619741, 4.677968, 1.712065}, {10.289137, 7.160522, -1.805385}, {9.940981, 7.770691, -1.380659}, {-2.120041, 7.106683, -0.173480},
},
{ // 7 : 10
{-3.206382, 5.365904, 2.713461}, {12.531212, 7.290931, -4.139583}, {9.736395, 6.192625, -3.211168}, {-0.808774, 7.464411, -0.683151}, {-4.664808, 8.009974, 0.800399},
{2.933600, 5.127820, 1.479961}, {11.454442, 7.607979, -2.586641}, {8.033901, 7.038487, -2.163112}, {-1.362712, 7.686942, -0.355334}, {-1.306481, 8.691928, 1.781456},
},
{ // 8 : 10
{-4.364508, 8.151150, 0.957129}, {9.333204, 8.337790, -0.252443}, {8.890532, 7.695317, -0.419940}, {-4.460221, 7.549356, -0.738186}, {1.598405, 7.000203, -0.252443},
{8.522038, 9.017352, 1.345963}, {2.496910, 8.901299, -0.053839}, {-0.832702, 9.029316, 0.662812}, {-0.570688, 9.365508, 1.395016}, {-0.244068, 9.688539, 1.416551},
},
{ // 9 : 10
{-3.330809, 9.231510, 0.416351}, {2.441875, 6.911668, 1.537389}, {10.674382, 8.793623, -0.124427}, {1.942972, 8.121240, -1.634298}, {-4.279563, 7.716853, -0.013161},
{0.428315, 6.967900, 0.727418}, {8.734999, 9.418149, 0.037089}, {5.035695, 9.007781, -0.417548}, {-1.374677, 8.394022, 0.646062}, {-0.878166, 9.153743, 1.534996},
},
{ // 10 : 10
{-4.297509, 7.952546, 1.599602}, {8.151150, 7.657032, -1.635494}, {11.949757, 7.990831, -1.161715}, {-0.959522, 7.045666, -1.478764}, {-5.868397, 7.222735, 0.472582},
{2.853441, 5.587241, 1.356731}, {11.580065, 8.148758, -1.221536}, {5.676972, 8.106883, -0.625723}, {-1.654637, 8.191828, -0.252443}, {-0.537189, 9.095119, 0.970290},
},
{ // 11 : 10
{-1.458425, 9.894321, 0.495314}, {-3.298506, 5.009374, 0.329013}, {13.183256, 7.235896, -0.719043}, {9.044869, 6.313462, -0.880559}, {-3.381058, 7.097112, -1.546960},
{-5.149354, 6.228518, -0.632902}, {8.689535, 6.995417, -0.930808}, {11.765509, 8.080562, -0.641276}, {2.730211, 7.789834, -1.428515}, {-1.307678, 8.288737, 0.288335},
},
{ // 12 : 10
{-3.701696, 9.164511, -0.057428}, {2.276771, 4.572684, 0.449851}, {14.292330, 7.313663, -0.977468}, {5.992825, 5.672186, -2.902494}, {-3.132205, 6.720243, -0.285942},
{-6.367301, 6.223732, 0.502493}, {9.097511, 6.577869, -0.927219}, {12.732209, 7.596015, -0.959522}, {1.058824, 7.098308, -0.898505}, {-0.948754, 8.402397, 1.118645},
},
{ // 13 : 10
{-4.615755, 3.948158, 1.274178}, {12.941580, 6.078966, -4.843073}, {11.728419, 6.126822, -2.408376}, {1.131805, 6.861418, -1.811367}, {-3.542574, 7.782655, -0.185444},
{-2.908476, 6.345766, -0.732204}, {12.681959, 8.132008, -1.219143}, {7.001399, 8.002795, -1.849652}, {-1.479961, 7.929814, 0.751346}, {-1.360320, 8.993423, 1.368694},
},
{ // 14 : 10
{-0.105284, 6.443871, 0.293121}, {10.230514, 8.288737, -1.993221}, {7.061220, 7.993224, -0.433101}, {-1.350748, 8.201399, 0.326620}, {-3.657429, 8.670393, -0.746561},
{3.250649, 6.471388, -0.118445}, {10.348958, 8.287540, -0.135194}, {6.184250, 8.276773, -0.606581}, {-0.172283, 8.453842, -0.056231}, {-0.781257, 8.981460, 0.841077},
},
{// 15 : 10
{-4.463811, 7.320841, 1.642673}, {7.197611, 7.917850, -1.106680}, {10.419547, 8.482556, -0.758525}, {-0.323031, 7.818548, -0.735793}, {-2.476571, 8.583055, 0.003589},
{-0.032303, 6.100501, 1.049253}, {10.463813, 8.721838, -0.380459}, {7.704889, 8.638089, -0.565903}, {-0.178265, 8.206184, -0.290728}, {-0.234497, 9.060423, 0.474975},
},
{ // 16 : 10
{-4.636094, 7.104290, 0.776471}, {9.239884, 8.148758, -1.478764}, {10.138390, 8.426324, -0.171087}, {-0.424726, 7.834101, -0.193819}, {-5.572884, 7.909475, 0.619741},
{2.333002, 6.697510, -0.363709}, {10.389636, 8.633304, -0.112463}, {5.613562, 8.376076, -0.430708}, {-1.155733, 8.195417, 0.220140}, {-0.738186, 9.219545, 0.713061},
},
{ // 17 : 10
{-4.468596, 8.889337, 1.001396}, {4.710271, 6.190232, -0.707079}, {12.178270, 7.701300, -1.222732}, {5.406582, 7.517052, -1.502693}, {-2.898905, 7.806584, -0.569492},
{-1.907080, 5.163712, -0.027517}, {9.869196, 7.877172, -0.733400}, {9.134600, 8.313863, -0.835095}, {0.002393, 8.072186, -0.530010}, {-0.561117, 8.782855, 0.756132},
},
{ // 18 : 10
{-3.139383, 5.759524, 1.009771}, {9.774680, 7.551748, -1.935793}, {10.707881, 7.446464, -2.647658}, {-1.619941, 7.135396, -0.287139}, {-4.198207, 5.699703, 0.028714},
{8.189436, 7.148558, -0.681954}, {11.937792, 8.274381, -0.803988}, {4.944768, 7.792227, -0.748954}, {-1.140180, 7.818548, -0.090927}, {-0.480957, 9.064012, 0.863809},
},
{ // 19 : 10
{-2.234896, 9.617950, 0.382852}, {-0.063410, 5.526224, 1.142573}, {11.392227, 8.082954, -1.586441}, {6.707082, 7.831708, -2.386841}, {-1.396212, 8.015956, -0.325424},
{-5.531009, 5.490331, 0.143569}, {9.042477, 7.221539, -0.843470}, {10.286745, 8.142776, -1.173679}, {3.437290, 7.889136, -2.100898}, {-1.189233, 7.847262, -0.208176},
},
{ // 20 : 10
{-2.202593, 9.507880, 0.131605}, {0.575474, 5.154140, 0.709472}, {11.566904, 7.950153, -0.692722}, {7.817351, 7.726424, -1.856830}, {-1.690529, 7.667800, -0.671187},
{-2.836691, 5.257031, -0.069392}, {9.467202, 7.529016, -1.771885}, {10.777272, 8.231310, -1.447658}, {3.035296, 7.655836, -0.759721}, {0.045464, 8.370094, 0.440279},
},
};



class k_means_clustering {
public:
	k_means_clustering();
	~k_means_clustering();

	bool get_template_base(int *template_base, float template_distance_table[][KCOUNT]);
	bool get_template_compare(int *template_compare, accel_t *accel_array);

private:
	mydata_t m_data[MCOUNT][NCOUNT];
	mydata_t m_means[KCOUNT];
	mydata_t m_histogram[KCOUNT][NCOUNT];
	mydata_t m_template_base[NCOUNT];
	bool     m_completed;

	void fill_data(void);
	void print_data(void);
	void decide_initial_means(void);
	void clustering(void);
	void decide_new_means(void);
	bool is_completed(void);
	void compute_template_base(void);
};

#endif /* _K_MEANS_CLUSTERING_H_ */

