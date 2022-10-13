#include "kernel.h"

#include <math.h>
#include <stdio.h>

#include "sycl.h"
#include <iostream>
#include <limits>

#include "BVH.h"
#include "Camera.h"
#include "Definitions.h"
#include "Disney.h"
#include "HDRI.h"
#include "Hit.h"
#include "Material.h"
#include "Math.hpp"
#include "PointLight.h"
#include "OslMaterial.hpp"
#include "lan/calc.tab.hpp"
#include "SYCLCopy.h"

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"


RngGenerator::RngGenerator(uint32_t _seed) {
    //this->seed = _seed + 1;
    this->seed = _seed + 1;
    this->state = _seed;

    for (int i = 0; i < _seed; i++) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
    }
}


const float RngGenerator::m_j[1024] = { 0.778805023064, 0.326792426673,
0.060188628771,
0.601773862531,
0.707751528929,
0.170931124466,
0.458919733624,
0.952535576650,
0.512585198132,
0.088331740814,
0.251516079619,
0.862908400701,
0.922391629098,
0.386548666625,
0.216826468750,
0.673837193632,
0.999021924676,
0.477059722559,
0.151758373405,
0.744262508044,
0.585673822187,
0.019285525676,
0.370631942989,
0.785200942347,
0.640531243101,
0.233338873831,
0.409579290947,
0.904403625285,
0.832662583709,
0.296902443637,
0.104768039936,
0.546699573603,
0.849545902707,
0.271678805199,
0.063771123409,
0.520464920445,
0.657655153192,
0.202596706940,
0.396562462913,
0.916283536345,
0.614171352907,
0.037984218255,
0.338949349675,
0.756518882671,
0.965889219837,
0.443103921160,
0.177728637280,
0.688106414427,
0.885647600983,
0.424736499956,
0.249402617996,
0.653112402477,
0.550250278501,
0.109655815748,
0.292902823545,
0.826504106506,
0.721441975144,
0.136607192708,
0.496659434321,
0.982408729598,
0.798208897035,
0.344537433209,
0.002740059403,
0.572017250577,
0.819693707429,
0.287296232248,
0.119118088405,
0.556518292224,
0.642267670276,
0.239355409988,
0.436059282053,
0.877507338447,
0.566917128734,
0.015238571620,
0.358531352366,
0.806652769892,
0.973208388754,
0.490830582633,
0.131971538949,
0.727583372424,
0.910946865087,
0.403916847708,
0.190638582374,
0.665806457148,
0.526616056151,
0.077402284828,
0.280933438516,
0.858468316040,
0.698527863919,
0.184469556130,
0.448001181960,
0.957492511346,
0.764275152946,
0.330748815282,
0.044428798850,
0.619502741416,
0.790870246141,
0.363181092314,
0.028261810774,
0.590896034109,
0.737896281224,
0.147446312148,
0.469453190115,
0.984962326381,
0.533676548644,
0.100940915836,
0.308869436131,
0.841441135412,
0.896125145796,
0.414984726903,
0.221155822416,
0.626575678093,
0.939923913474,
0.463217872897,
0.157494568041,
0.712967893363,
0.599347129668,
0.048259935988,
0.317814795929,
0.766192395895,
0.683669570552,
0.209891993783,
0.377597737680,
0.930233124821,
0.872519616484,
0.259718833775,
0.082579540457,
0.504763283382,
0.863920912978,
0.256540313386,
0.090994906324,
0.509935001666,
0.677894426221,
0.214678860029,
0.387460624480,
0.926005502197,
0.605868702531,
0.055581262240,
0.323839422138,
0.776437015859,
0.948995439402,
0.454557818200,
0.164888086547,
0.705093320818,
0.901354565137,
0.411702439550,
0.226863474217,
0.635909024121,
0.541706641859,
0.105492093262,
0.301647573008,
0.829907805452,
0.746824647637,
0.155477193934,
0.482223352854,
0.994898523894,
0.781371745959,
0.372145473317,
0.023156047779,
0.580297635519,
0.750335535180,
0.340600652939,
0.032309425816,
0.611872830717,
0.692538722726,
0.173103974396,
0.439126595799,
0.962193220807,
0.516314325783,
0.069028599309,
0.269064303752,
0.845657918686,
0.920033734172,
0.392578691980,
0.195595630029,
0.661373478165,
0.980004846419,
0.492461959739,
0.139499052109,
0.723369515448,
0.577892856166,
0.005223190256,
0.349700325254,
0.801109496980,
0.650976406037,
0.245015644095,
0.429459285585,
0.889616060918,
0.822662580319,
0.293653772734,
0.116610855006,
0.554254488095,
0.810561980642,
0.351598902125,
0.009874955283,
0.563698215277,
0.731072734516,
0.128457781391,
0.485340237342,
0.970146216047,
0.561976779156,
0.122690325309,
0.282003521956,
0.812563843696,
0.880774203201,
0.430173066296,
0.236395033662,
0.646003400612,
0.956223644344,
0.450154358104,
0.181926907660,
0.701266298106,
0.621743941162,
0.039940835798,
0.333527119076,
0.760865401125,
0.669875534279,
0.194650475651,
0.401417490659,
0.908495536767,
0.852786031843,
0.275799036862,
0.071771017501,
0.529225407217,
0.837246270892,
0.305416323648,
0.095013186373,
0.535394798396,
0.632118892480,
0.224683865337,
0.420283241470,
0.893030479469,
0.588625107990,
0.024660305753,
0.364640080991,
0.795191606154,
0.990286089398,
0.472702883369,
0.144514047392,
0.740295546954,
0.935699041065,
0.380143287750,
0.205655800733,
0.680286158261,
0.500938815458,
0.081895763427,
0.262471620998,
0.870593123445,
0.714891923368,
0.162114995054,
0.468630079255,
0.941803219863,
0.771269308409,
0.312603339910,
0.052918791887,
0.597317559094,
0.787142740232,
0.368617785617,
0.016020507884,
0.583710924472,
0.743939847098,
0.149333383744,
0.479140025339,
0.996223022769,
0.543983342366,
0.102822223981,
0.299779272897,
0.835139744258,
0.902743045867,
0.406639541264,
0.231971541751,
0.637507049600,
0.949815336620,
0.460280115333,
0.169554822833,
0.710095885892,
0.605393155336,
0.061448713415,
0.325041599047,
0.780323390582,
0.672402018000,
0.215667450278,
0.383595740788,
0.924696593752,
0.860645433286,
0.253172298990,
0.087403318483,
0.515571656978,
0.825348394022,
0.289781573239,
0.111968643637,
0.547599615460,
0.654423955651,
0.246903123139,
0.422549367047,
0.884021009961,
0.573839580208,
0.000912732588,
0.345979570761,
0.798952214838,
0.983547033592,
0.498104057085,
0.133447711792,
0.720408531053,
0.915618194006,
0.396341516567,
0.199855752035,
0.658682976536,
0.522894069391,
0.065505902856,
0.270962880130,
0.850168536473,
0.691102181631,
0.179319041464,
0.443787356433,
0.968028270518,
0.754507400766,
0.336067826000,
0.035239796045,
0.615778973691,
0.856327292275,
0.277726832483,
0.075834419382,
0.525087614904,
0.666850729725,
0.188303684480,
0.405048482433,
0.913763224263,
0.618325989263,
0.046705331729,
0.329333567605,
0.762181630511,
0.960626863236,
0.445497669740,
0.187291825936,
0.696167295689,
0.875850730065,
0.434970296430,
0.241421716991,
0.643653057252,
0.556830180420,
0.121059115942,
0.285301526040,
0.816545392976,
0.730225963926,
0.129025399038,
0.489508111395,
0.976548315599,
0.804868973971,
0.357040172464,
0.012562349155,
0.569426305843,
0.768121447361,
0.319667974197,
0.049032539365,
0.600049260016,
0.711232577230,
0.158276273114,
0.461285985685,
0.938344537651,
0.506882318891,
0.084683301482,
0.260375687333,
0.873334559627,
0.933381527238,
0.376369391535,
0.208743743924,
0.687389480469,
0.987526367827,
0.470911003830,
0.146176700466,
0.734893593964,
0.593749605534,
0.029993958787,
0.360941163315,
0.792344318771,
0.627014883419,
0.219374314664,
0.416781788842,
0.897912686925,
0.842486995068,
0.311821886547,
0.099286221003,
0.531389433608,
0.831902571826,
0.304366161264,
0.109201007584,
0.539597865653,
0.634089430540,
0.229505208154,
0.413789231847,
0.899346317744,
0.578700618868,
0.020291579074,
0.373442193729,
0.783847284692,
0.993962194137,
0.483754260501,
0.153059368504,
0.748375269890,
0.928027928457,
0.388972992595,
0.211985949536,
0.677312821737,
0.508286310351,
0.093516891711,
0.254546336812,
0.865256799985,
0.704836678440,
0.167885946997,
0.456563216477,
0.946178611629,
0.775330949633,
0.321587280355,
0.056913132594,
0.608861565056,
0.803466267628,
0.347688042007,
0.006384879286,
0.575489825482,
0.724863637713,
0.137244950284,
0.494964553637,
0.978193947621,
0.551923412324,
0.113349283128,
0.296771679233,
0.821154979904,
0.888382759157,
0.426979365439,
0.243306465641,
0.649995271979,
0.963379317708,
0.441369329678,
0.175656936254,
0.694404740056,
0.610710241113,
0.034841229719,
0.342088247239,
0.752184852413,
0.664003178601,
0.198493818424,
0.392226834158,
0.917996293137,
0.847278437041,
0.266456753334,
0.066698206806,
0.518475118240,
0.758755941885,
0.334162051364,
0.041864418931,
0.624677600219,
0.700302988758,
0.180371461815,
0.451936188204,
0.954647532635,
0.529451336173,
0.074038750590,
0.274740903537,
0.853847336166,
0.906414294171,
0.399945085076,
0.192121449909,
0.670073953663,
0.972245383577,
0.487675127388,
0.126629691323,
0.734188191459,
0.565583298189,
0.008218434687,
0.353828894422,
0.809923154437,
0.647869839446,
0.234578227456,
0.432961742805,
0.881336578110,
0.816353910492,
0.283325843709,
0.123577127943,
0.558838847757,
0.868594938751,
0.265320903700,
0.079174646425,
0.503874056424,
0.682608175827,
0.203327307849,
0.381767887675,
0.935154065084,
0.594979812791,
0.051347496618,
0.314694097160,
0.772185534608,
0.943902922278,
0.465326586395,
0.160509863993,
0.716980670905,
0.890925122287,
0.418087705025,
0.222872123692,
0.630553456205,
0.538502747099,
0.096255498436,
0.308390570829,
0.839566208766,
0.739099627408,
0.141969691407,
0.475133410522,
0.988461684310,
0.793417786529,
0.366572676316,
0.025490279690,
0.587721028101,
0.757455703550,
0.337958489532,
0.038652020482,
0.615082824917,
0.688847248337,
0.176606237830,
0.442235783313,
0.965751217447,
0.520631257859,
0.062639874908,
0.272867796075,
0.847807695784,
0.917870299463,
0.398181012356,
0.201210282278,
0.656505509723,
0.981110994940,
0.497695130816,
0.135364450768,
0.722526239252,
0.570946346434,
0.003868578778,
0.345036989490,
0.797179193263,
0.654042691838,
0.248409704447,
0.424952825119,
0.886427634132,
0.827907767617,
0.291070046899,
0.110440474907,
0.548838329373,
0.861875066347,
0.250507821951,
0.088961400167,
0.513311734750,
0.675329783721,
0.218250530155,
0.384790292127,
0.923709194652,
0.603155606385,
0.059404764996,
0.327513873728,
0.777893452061,
0.951406360286,
0.457882909156,
0.170192271368,
0.708954761291,
0.905547023056,
0.408219660520,
0.234284516132,
0.639280467201,
0.545127979547,
0.104270410756,
0.298418890642,
0.833770096268,
0.745581619701,
0.151056195608,
0.477941196136,
0.999316627205,
0.786570128792,
0.369300685250,
0.018227217067,
0.584584214166,
0.840168018432,
0.310521483243,
0.100537534989,
0.534720981150,
0.625261825141,
0.222611410076,
0.415457882898,
0.895469662415,
0.590717557257,
0.028846173385,
0.361653811406,
0.789748156926,
0.985730601582,
0.470168720235,
0.147822154520,
0.737182672425,
0.930942748952,
0.378812658206,
0.210565960629,
0.684848455802,
0.505162533728,
0.083787794675,
0.257924637904,
0.871411269383,
0.714016964829,
0.156336633906,
0.463970052851,
0.940510995361,
0.766829634343,
0.317023447342,
0.047456725563,
0.597743917098,
0.808122847472,
0.357729813209,
0.014097408547,
0.567582154580,
0.726736641696,
0.131529014727,
0.492095716025,
0.973816830878,
0.554843279840,
0.117755366673,
0.288381410927,
0.819067599313,
0.878877960874,
0.437410821512,
0.238353459229,
0.641568223983,
0.958795219164,
0.448427536633,
0.185183507733,
0.698053084191,
0.620638260400,
0.043342584191,
0.331802546967,
0.765542391992,
0.664806161847,
0.190126687156,
0.402791976678,
0.911243024974,
0.857745760475,
0.279657456460,
0.076314933663,
0.525765578880,
0.844507870825,
0.268092753039,
0.069932808066,
0.516682693117,
0.660406003505,
0.196572848472,
0.394312175003,
0.921834336128,
0.612506363269,
0.032148079483,
0.341658709481,
0.751635319881,
0.961523450266,
0.437613935895,
0.172468640897,
0.691869892119,
0.889761585405,
0.428177814430,
0.245464504220,
0.652309531023,
0.553563449728,
0.115676750826,
0.294479907170,
0.823753829991,
0.723642505606,
0.140226756619,
0.494007109663,
0.979477674088,
0.802598066657,
0.351511962787,
0.004403466706,
0.576185322995,
0.775750783145,
0.322411403258,
0.056579637742,
0.606874218666,
0.706088047441,
0.165487319315,
0.454077550692,
0.947405820940,
0.511260317098,
0.090196082096,
0.257472851623,
0.864558273924,
0.926820228901,
0.388610071032,
0.213558182269,
0.679001249068,
0.995431443972,
0.481105782065,
0.155208653240,
0.747388134141,
0.582018897202,
0.022379026171,
0.371797409156,
0.782336471947,
0.635424435273,
0.228425439161,
0.410617480824,
0.901914502537,
0.828868925918,
0.302425284615,
0.107164042025,
0.542835019843,
0.796167875271,
0.363953010498,
0.023972718282,
0.588923625513,
0.741834918628,
0.142612621589,
0.473847884624,
0.992101377598,
0.536303938429,
0.094464704252,
0.306603244576,
0.836684034312,
0.894073734860,
0.421296831935,
0.225801827066,
0.630911327439,
0.942484011576,
0.467438718769,
0.163724351565,
0.716001227166,
0.596403875471,
0.054686520005,
0.314403871493,
0.770491411896,
0.681535921182,
0.206149846094,
0.379408633941,
0.936673925901,
0.869672529099,
0.263177382674,
0.080703250021,
0.501690070608,
0.813498330053,
0.282418555409,
0.121097989208,
0.560857665260,
0.645464535170,
0.237459515605,
0.431467053174,
0.879022505910,
0.562978232612,
0.011653664619,
0.353046546190,
0.811700108909,
0.969019268292,
0.486301407063,
0.127148318914,
0.732320115948,
0.910155648443,
0.400989323766,
0.194118941529,
0.668426169481,
0.528029155197,
0.071019953171,
0.276487724605,
0.852150720658,
0.702938957722,
0.183397576973,
0.451062531178,
0.955783287568,
0.760620885504,
0.332549107481,
0.040337379317,
0.622672632990,
0.800251648652,
0.347037156146,
0.001396481938,
0.572724159240,
0.719274877279,
0.134104572464,
0.499908869841,
0.982952469508,
0.548345403182,
0.112425449107,
0.290632101710,
0.825174436621,
0.883262957957,
0.423057958940,
0.247284599326,
0.655473782584,
0.967644302169,
0.444852479651,
0.177913293682,
0.689682079226,
0.616419367903,
0.036630825474,
0.336971938406,
0.755695915724,
0.659803159834,
0.200935516410,
0.395053819379,
0.914688003324,
0.851157798964,
0.270125379911,
0.064781646766,
0.522099731742,
0.834418618471,
0.300766007882,
0.101947238702,
0.543934253355,
0.638390655140,
0.231288111269,
0.407900097500,
0.903531972397,
0.582859308432,
0.017544843370,
0.368084970859,
0.788133904044,
0.997662103989,
0.479968891357,
0.150177764066,
0.742439957928,
0.925480458866,
0.384647438946,
0.216657747143,
0.673167554387,
0.514313377534,
0.086629697661,
0.252399320152,
0.860001943043,
0.709568871702,
0.168178885508,
0.459804397874,
0.950956372059,
0.780131416137,
0.325808602119,
0.062363091218,
0.603865833913,
0.874800339277,
0.261523273669,
0.085778014084,
0.506710172322,
0.686353370806,
0.207283635390,
0.375205799305,
0.932586859619,
0.601385261556,
0.049811949195,
0.318956977046,
0.768818288100,
0.938930174194,
0.461940105985,
0.160067086281,
0.712825574049,
0.896566684083,
0.417568365538,
0.219803098873,
0.628188620469,
0.532466800521,
0.098035536952,
0.311323034678,
0.843357472646,
0.736068420982,
0.144990164727,
0.472356448476,
0.987247537213,
0.791171585831,
0.359800992726,
0.030501070988,
0.591951416783,
0.763294802272,
0.328220304679,
0.045096821495,
0.617537854682,
0.696296085307,
0.186125633538,
0.446979910102,
0.959283765697,
0.523598663702,
0.074896751888,
0.278648735164,
0.856893892690,
0.912764309499,
0.406141097817,
0.188879383594,
0.667259937198,
0.975252468231,
0.488507991209,
0.130313090959,
0.729338913192,
0.568712976808,
0.013148474389,
0.356125550544,
0.806126641353,
0.643440628579,
0.240561785306,
0.433912043311,
0.876752812346,
0.817764120316,
0.286561781620,
0.119164575247,
0.558150635152,
0.821949138725,
0.295013892579,
0.114558419255,
0.551313974397,
0.649263144343,
0.242635946412,
0.426235510659,
0.887481296259,
0.575100954188,
0.007495514693,
0.349538834522,
0.803776148535,
0.977270698822,
0.495801655741,
0.138230938883,
0.725992686087,
0.919842496908,
0.391021828792,
0.197603686481,
0.662253535137,
0.519437989407,
0.067540664662,
0.267537193537,
0.846052973704,
0.693614938606,
0.174384268309,
0.439503650975,
0.964625189025,
0.753123981597,
0.343160224018,
0.033626508355,
0.609468429454,
0.785119398907,
0.374119815270,
0.021165474817,
0.579539847453,
0.749278625558,
0.153625420781,
0.483207459380,
0.992536908094,
0.540531633075,
0.107488347171,
0.303676879992,
0.830890203865,
0.900276777235,
0.412539874481,
0.228854372900,
0.633372093605,
0.946740299323,
0.455917690989,
0.166498303822,
0.703919253123,
0.607905899962,
0.057885190011,
0.320449505308,
0.774040530453,
0.675836877601,
0.211726476192,
0.389957627903,
0.929612436269,
0.866499865870,
0.255435643663,
0.092681745341,
0.509623968736,
0.772891741994,
0.316084337739,
0.051891836697,
0.594100659342,
0.718097593375,
0.161816762524,
0.466115470859,
0.944532780991,
0.502194655306,
0.078231628651,
0.263771526560,
0.867268252062,
0.934137241187,
0.382626799159,
0.204706495546,
0.682941512757,
0.989854290477,
0.476051398477,
0.141054801631,
0.739629309974,
0.586598254915,
0.027151126390,
0.366129699864,
0.794234348799,
0.629690798447,
0.224043889365,
0.419609131265,
0.891870106354,
0.838162137068,
0.307176784416,
0.097029450121,
0.537350533963,
0.855060163072,
0.273670223210,
0.072841755437,
0.530925727919,
0.671618275299,
0.192802985702,
0.399293877443,
0.907959343526,
0.623980800337,
0.042880219966,
0.335496735289,
0.759762814611,
0.953350033346,
0.452519612129,
0.181250118246,
0.699351182155,
0.882274946274,
0.431679123100,
0.236197167418,
0.647356394032,
0.560269821320,
0.124847884042,
0.284656526361,
0.814994869107,
0.732610141643,
0.125130007992,
0.486404505323,
0.971197022761,
0.808866745216,
0.354709129855,
0.009337925071,
0.564559743996 };

float RngGenerator::next() {
    //seed ^= seed << 13;
    //seed ^= seed >> 17;
    //seed ^= seed << 5;
    //return ((float)seed / (float)UINT_MAX);
    return m_j[(state++)%1024];
}

//TODO move this function
Passes parsePass(std::string s_pass) {

    std::transform(s_pass.begin(), s_pass.end(), s_pass.begin(), ::tolower);

    Passes pass = BEAUTY;

    if (s_pass.compare("beauty") == 0)
        pass = BEAUTY;
    if (s_pass.compare("denoise") == 0)
        pass = DENOISE;
    if (s_pass.compare("normal") == 0)
        pass = NORMAL;
    if (s_pass.compare("tangent") == 0)
        pass = TANGENT;
    if (s_pass.compare("bitangent") == 0)
        pass = BITANGENT;

    return pass;
}

unsigned long textureMemory = 0;
unsigned long geometryMemory = 0;

void generateHitData(dev_Scene* dev_scene_g, Material* material,
                     HitData& hitdata, Hit hit) {
    Vector3 tangent, bitangent, normal;

    normal = hit.normal;
    tangent = hit.tangent;
    bitangent = hit.bitangent;

    if (material->albedoTextureID < 0) {
        hitdata.albedo = material->albedo;
    } else {
        hitdata.albedo = dev_scene_g->textures[material->albedoTextureID]
                             .getValueFromUVFiltered(hit.tu, hit.tv);
    }

    if (material->opacityTextureID < 0) {
        hitdata.opacity = material->opacity;
    }
    else {
        hitdata.opacity = dev_scene_g->textures[material->opacityTextureID]
            .getValueFromUVFiltered(hit.tu, hit.tv).x;
    }

    if (material->emissionTextureID < 0) {
        hitdata.emission = material->emission;
    } else {
        hitdata.emission = dev_scene_g->textures[material->emissionTextureID]
                               .getValueFromUVFiltered(hit.tu, hit.tv);
    }

    if (material->roughnessTextureID < 0) {
        hitdata.roughness = material->roughness;
    } else {
        hitdata.roughness = dev_scene_g->textures[material->roughnessTextureID]
                                .getValueFromUVFiltered(hit.tu, hit.tv)
                                .x;
    }

    if (material->metallicTextureID < 0) {
        hitdata.metallic = material->metallic;
    } else {
        hitdata.metallic = dev_scene_g->textures[material->metallicTextureID]
                               .getValueFromUVFiltered(hit.tu, hit.tv)
                               .x;
    }

    if (material->normalTextureID < 0) {
        hitdata.normal = normal;
    } else {
        Vector3 ncolor =
            dev_scene_g->textures[material->normalTextureID].getValueFromUV(
                hit.tu, hit.tv);
        Vector3 localNormal = (ncolor * 2) - 1;
        Vector3 worldNormal =
            (localNormal.x * tangent - localNormal.y * bitangent +
             localNormal.z * normal)
                .normalized();

        hitdata.normal = worldNormal;
    }

    // Convert linear to sRGB
    // TODO: move to texture loading
    hitdata.roughness = sycl::pow(hitdata.roughness, 2.2f);
    hitdata.metallic = sycl::pow(hitdata.metallic, 2.2f);

    hitdata.clearcoatGloss = material->clearcoatGloss;
    hitdata.clearcoat = material->clearcoat;
    hitdata.anisotropic = material->anisotropic;
    hitdata.eta = material->eta;
    hitdata.transmission = material->transmission;
    hitdata.specular = material->specular;
    hitdata.specularTint = material->specularTint;
    hitdata.sheenTint = material->sheenTint;
    hitdata.subsurface = material->subsurface;
    hitdata.sheen = material->sheen;

    hitdata.tangent = tangent;
    hitdata.bitangent = bitangent;
}


void thread_count_kernel(int* c, int idx, sycl::stream out) {

    (*c)++;
    int i = 0;
    while(i < 1000000) {
        i++;
    }

    out << idx << ": " << (*c) << " - " << i << "\n";
    (*c)--;
}

void setupKernel(dev_Scene* dev_scene_g, int idx, sycl::stream out, OslMaterial* dev_osl) {

    dev_scene_g->dev_randstate[idx] = RngGenerator(idx);

    for (int i = 0; i < PASSES_COUNT; i++) {
        dev_scene_g->dev_passes[(i * dev_scene_g->camera->xRes * dev_scene_g->camera->yRes *
                    4) +
                   (4 * idx + 0)] = 0.0;
        dev_scene_g->dev_passes[(i * dev_scene_g->camera->xRes * dev_scene_g->camera->yRes *
                    4) +
                   (4 * idx + 1)] = 0.0;
        dev_scene_g->dev_passes[(i * dev_scene_g->camera->xRes * dev_scene_g->camera->yRes *
                    4) +
                   (4 * idx + 2)] = 0.0;
        dev_scene_g->dev_passes[(i * dev_scene_g->camera->xRes * dev_scene_g->camera->yRes *
                    4) +
                   (4 * idx + 3)] = 1.0;
    }

    dev_scene_g->dev_samples[idx] = 0;

    if (idx == 0) {

        out << "PRINTING EMPTY\n";

        dev_osl->program->print(out);

        out << "COMPUTING\n";

        dev_osl->compute_ci(out);

        out << "PRINTING FULL\n";

        //dev_osl->program->print(out);

        dev_osl->vars.print(out);

        int triSum = 0;
        for (int i = 0; i < dev_scene_g->meshObjectCount; i++) {
            dev_scene_g->meshObjects[i].tris += triSum;
            triSum += dev_scene_g->meshObjects[i].triCount;
        }
    }
}

Hit throwRay(Ray ray, dev_Scene* scene) {
    Hit nearestHit = Hit();

#if USEBVH
    scene->bvh->transverse(ray, nearestHit);
#else
    for (int j = 0; j < scene->meshObjectCount; j++) {
        Hit hit = Hit();

        if (scene->meshObjects[j].hit(ray, hit)) {
            if (!nearestHit.valid) nearestHit = hit;

            if ((hit.position - ray.origin).length() <
                (nearestHit.position - ray.origin).length())
                nearestHit = hit;
        }
    }
#endif
    return nearestHit;
}


dev_Scene::dev_Scene(Scene* scene) {

    camera = &(scene->camera);

    meshObjectCount = scene->meshObjectCount();
    materialCount = scene->materialCount();
    textureCount = scene->textureCount();
    triCount = scene->triCount();
    pointLightCount = scene->pointLightCount();

    pointLights = scene->getPointLights();
    meshObjects = scene->getMeshObjects();
    materials = scene->getMaterials();
    textures = scene->getTextures();
    hdri = &scene->hdri;

    tris = scene->getTris();
    bvh = scene->buildBVH();

    //float* dev_passes;
    //unsigned int* dev_samples;
    //RngGenerator* dev_randstate;
}


Vector3 pointLight(Ray ray, HitData hitdata, dev_Scene* scene, Vector3 point,
                   float& pdf, float r1) {
    if (scene->pointLightCount <= 0) {
        pdf = 0;
        return Vector3::Zero();
    }

    pdf = ((float)scene->pointLightCount) / (2.0 * PI);

    // Retrieve a random light
    PointLight light = scene->pointLights[(int)(scene->pointLightCount * r1)];

    Vector3 newDir = (light.position - point).normalized();

    float dist = (light.position - point).length();

    // Test if the point is visible from the light
    Ray shadowRay(point + newDir * 0.001, newDir);
    Hit shadowHit = throwRay(shadowRay, scene);
    float shadowDist = (shadowHit.position - point).length();

    if (shadowHit.valid && shadowDist < dist) return Vector3();

    // Quadratic attenuation
    Vector3 pointLightValue = (light.radiance / (dist * dist));

    //Vector3 brdfDisney = DisneyEval(ray, hitdata, newDir);

    //return pointLightValue * brdfDisney *
    //       abs(Vector3::dot(newDir, hitdata.normal)) / pdf;
}

// Sampling HDRI
// The main idea is to get a random point of the HDRI, weighted by their
// importance and then get the direction from the center to that point as if
// that pixel would be in a sphere of infinite radius.
Vector3 hdriLight(Ray ray, dev_Scene* scene, Vector3 point, HitData hitdata, RngGenerator& rnd, float& pdf) {
    if (!HDRIIS) {

        Vector3 newDir = DisneySample(ray, hitdata, rnd.next(), rnd.next(), rnd.next());
       
        //Vector3 newDir = uniformSampleSphere(rnd.next(), rnd.next()).normalized();

        //if (Vector3::dot(newDir, hitdata.normal) < 0)
        //    newDir *= -1;

        float u, v;

        Texture::sphericalMapping(Vector3(), -1 * newDir, 1, u, v);

        Ray shadowRay(point + hitdata.normal * 0.0001, newDir);

        Hit shadowHit = throwRay(shadowRay, scene);

        if (shadowHit.valid)
            return Vector3();

        Vector3 hdriValue = scene->hdri->texture.getValueFromUV(u, v);
        Vector3 disneyBrdf = DisneyEval(ray, hitdata, newDir);

        // This should be 1/2pi, but is so dim with it :/
        //pdf = 1 / (2.0 * 2.0 * PI);
        pdf = DisneyPdf(ray, hitdata, newDir);

        return hdriValue * disneyBrdf * abs(Vector3::dot(hitdata.normal, newDir)) / pdf;

    } else {
        
        Vector3 textCoordinate = scene->hdri->sample(rnd.next());

        float nu = textCoordinate.x / (float)scene->hdri->texture.width;
        float nv = textCoordinate.y / (float)scene->hdri->texture.height;

        float iu = scene->hdri->texture.inverseTransformUV(nu, nv).x;
        float iv = scene->hdri->texture.inverseTransformUV(nu, nv).y;

        Vector3 newDir =
            -scene->hdri->texture.reverseSphericalMapping(iu, iv).normalized();

        Ray shadowRay(point + newDir * 0.0001, newDir);

        Hit shadowHit = throwRay(shadowRay, scene);

        if (shadowHit.valid) return Vector3();

        Vector3 hdriValue = scene->hdri->texture.getValueFromUV(iu, iv);

        Vector3 brdfDisney = DisneyEval(ray, hitdata, newDir);

        pdf = scene->hdri->pdf(iu * scene->hdri->texture.width,
                               iv * scene->hdri->texture.height);

        return brdfDisney * abs(Vector3::dot(newDir, hitdata.normal)) *
               (hdriValue / pdf);
    }
}

void calculateCameraRay(int x, int y, Camera& camera, Ray& ray, float r1,
                        float r2, float r3, float r4, float r5) {
    // Relative coordinates for the point where the first ray will be launched
    float dx = camera.position.x +
               ((float)x) / ((float)camera.xRes) * camera.sensorWidth;
    float dy = camera.position.y +
               ((float)y) / ((float)camera.yRes) * camera.sensorHeight;

    // Absolute coordinates for the point where the first ray will be launched
    float odx = (-camera.sensorWidth / 2.0) + dx;
    float ody = (-camera.sensorHeight / 2.0) + dy;

    // Random part of the sampling offset so we get antialasing
    float rx = (1.0 / (float)camera.xRes) * (r1 - 0.5) * camera.sensorWidth;
    float ry = (1.0 / (float)camera.yRes) * (r2 - 0.5) * camera.sensorHeight;

    // Sensor point, the point where intersects the ray with the sensor
    float SPx = odx + rx;
    float SPy = ody + ry;
    float SPz = camera.position.z + camera.focalLength;

    /*
    | 1     0           0| |x|   |        x         |   |x'|
    | 0   cos θ    −sin θ| |y| = | y cos θ − z sin θ| = |y'|
    | 0   sin θ     cos θ| |z|   |y sin θ + z cos θ |   |z'|
    */

    /*
    | cos θ    0    sin θ| |x|   | x cos θ + z sin θ|   |x'|
    | 0        1        0| |y| = | y                | = |y'|
    | -sin θ   0    cos θ| |z|   | z cos θ - x sin θ|   |z'|
    */

    /*
    |cos θ   −sin θ   0| |x|   |x cos θ − y sin θ|   |x'|
    |sin θ    cos θ   0| |y| = |x sin θ + y cos θ| = |y'|
    |  0       0      1| |z|   |        z        |   |z'|
    */

    // XYZ Eulers's Rotation
    // TODO CLEANUP AND PRECALC
    Vector3 rotation = camera.rotation;

    rotation *= (PI / 180.0);

    Vector3 dir = Vector3(SPx, SPy, SPz) - camera.position;
    Vector3 dirXRot = Vector3(
        dir.x, dir.y * sycl::cos(rotation.x) - dir.z * sycl::sin(rotation.x),
        dir.y * sycl::sin(rotation.x) + dir.z * sycl::cos(rotation.x));
    Vector3 dirYRot = Vector3(
        dirXRot.x * sycl::cos(rotation.y) + dirXRot.z * sycl::sin(rotation.y),
        dirXRot.y,
        dirXRot.z * sycl::cos(rotation.y) - dirXRot.x * sycl::sin(rotation.y));
    Vector3 dirZRot = Vector3(
        dirYRot.x * sycl::cos(rotation.z) - dirYRot.y * sycl::sin(rotation.z),
        dirYRot.x * sycl::sin(rotation.z) + dirYRot.y * sycl::cos(rotation.z),
        dirYRot.z);

    // The initial ray is created from the camera position to the sensor point.
    // No rotation is taken into account.
    ray = Ray(camera.position, dirZRot);

    if (camera.bokeh) {
        float rIPx, rIPy;

        // The diameter of the camera iris
        float diameter = camera.focalLength / camera.aperture;

        // Total length from the camera to the focus plane
        float l = camera.focusDistance + camera.focalLength;

        // The point from the initial ray which is actually in focus
        // Vector3 focusPoint = ray.origin + ray.direction * (l /
        // (ray.direction.z));
        // Mala aproximación, encontrar soluición
        Vector3 focusPoint = ray.origin + ray.direction * l;

        // Sampling for the iris of the camera
        uniformCircleSampling(r3, r4, r5, rIPx, rIPy);

        rIPx *= diameter * 0.5;
        rIPy *= diameter * 0.5;

        Vector3 orig = camera.position + Vector3(rIPx, rIPy, 0);

        // Blurred ray
        ray = Ray(orig, focusPoint - orig);
    }
}



void shade(dev_Scene& scene, Ray& ray, HitData& hitdata, Hit& nearestHit,
           Vector3& newDir, RngGenerator& rnd, Vector3& hitLight,
           Vector3& reduction, int idx) {

  
    //Vector3 brdfDisney = DisneyEval(ray, hitdata, newDir);

    //float brdfPdf = DisneyPdf(ray, hitdata, newDir);
    float hdriPdf;
    float pointPdf = 0;

    Vector3 hdriLightCalc = hdriLight(ray, &scene, nearestHit.position, hitdata, rnd, hdriPdf);

    Vector3 pointLightCalc =
        pointLight(ray, hitdata, &scene, nearestHit.position, pointPdf, rnd.next());

    //Vector3 brdfLightCalc =
    //    hitdata.emission *
    //    (brdfDisney * abs(Vector3::dot(newDir, hitdata.normal))) / brdfPdf;

    //float w1 = hdriPdf / (hdriPdf + pointPdf + brdfPdf);
    //float w2 = pointPdf / (hdriPdf + pointPdf + brdfPdf);
    //float w3 = brdfPdf / (hdriPdf + pointPdf + brdfPdf);

    //hitLight = reduction * (w1 * hdriLightCalc + w2 * pointLightCalc + w3 * brdfLightCalc);
    hitLight = reduction * hdriLightCalc;

    //reduction *= (brdfDisney * abs(Vector3::dot(newDir, hitdata.normal))) / brdfPdf;

}

void calculateBounce(Ray& incomingRay, HitData& hitdata, Vector3& bouncedDir,
                     float r1, float r2, float r3) {
    //bouncedDir = DisneySample(incomingRay, hitdata, r1, r2, r3);
}



void renderingKernel(dev_Scene* scene, int idx, int s) {

    RngGenerator rnd = scene->dev_randstate[idx];
    
    unsigned int sa = scene->dev_samples[idx];
    
    Ray ray;
    
    int x = (idx % scene->camera->xRes);
    int y = (idx / scene->camera->xRes);


    calculateCameraRay(x, y, *scene->camera, ray, rnd.next(), rnd.next(),
        rnd.next(), rnd.next(), rnd.next());
    
    // Accumulated radiance
    Vector3 light = Vector3::Zero();
    
    // Accumulated radiance
    Vector3 normal = Vector3::Zero();
    Vector3 tangent = Vector3::Zero();
    Vector3 bitangent = Vector3::Zero();
    
    // How much light is lost in the path
    Vector3 reduction = Vector3::One();
    
    int i = 0;
    
    for (i = 0; i < MAXBOUNCES; i++) {
    
        Vector3 hitLight;
        HitData hitdata;
        Vector3 bouncedDir;
    
        int materialID = 0;
    
        Hit nearestHit = throwRay(ray, scene);
    
        if (!nearestHit.valid) {
            float u, v;
            Texture::sphericalMapping(Vector3(), -1 * ray.direction, 1, u, v);
            light +=
                scene->hdri->texture.getValueFromUVFiltered(u, v) * reduction;
            break;
        }
    
        materialID = scene->meshObjects[nearestHit.objectID].materialID;
    
        Material* material = &scene->materials[materialID];
    
        generateHitData(scene, material, hitdata, nearestHit);

        if (rnd.next() <= hitdata.opacity) {
            calculateBounce(ray, hitdata, bouncedDir, rnd.next(), rnd.next(),
                rnd.next());

            shade(*scene, ray, hitdata, nearestHit, bouncedDir, rnd, hitLight, reduction, idx);

            light += hitLight;

            // First hit
            if (i == 0) {
                normal = hitdata.normal;
                tangent = hitdata.tangent;
                bitangent = hitdata.bitangent;
            }

            ray = Ray(nearestHit.position + bouncedDir * 0.0001, bouncedDir);
        }
        else {
            ray = Ray(nearestHit.position + ray.direction * 0.0001, ray.direction);
        }
    }
    
    // TODO: parametrize light clamp
    light = clamp(light, 0, 10);
    
    if (!sycl::isnan(light.x) && !sycl::isnan(light.y) &&
        !sycl::isnan(light.z)) {
        if (sa > 0) {
            for (int pass = 0; pass < PASSES_COUNT; pass++) {
                if (pass != DENOISE) {
                    scene->dev_passes[(pass * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] *=
                        ((float)sa) / ((float)(sa + 1));
                    scene->dev_passes[(pass * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] *=
                        ((float)sa) / ((float)(sa + 1));
                    scene->dev_passes[(pass * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] *=
                        ((float)sa) / ((float)(sa + 1));
                }
            }
        }

        scene->dev_passes[(BEAUTY * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] +=
            light.x / ((float)sa + 1);
        scene->dev_passes[(BEAUTY * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] +=
            light.y / ((float)sa + 1);
        scene->dev_passes[(BEAUTY * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] +=
            light.z / ((float)sa + 1);

        scene->dev_passes[(NORMAL * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] +=
            normal.x / ((float)sa + 1);
        scene->dev_passes[(NORMAL * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] +=
            normal.y / ((float)sa + 1);
        scene->dev_passes[(NORMAL * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] +=
            normal.z / ((float)sa + 1);

        scene->dev_passes[(TANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] +=
            tangent.x / ((float)sa + 1);
        scene->dev_passes[(TANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] +=
            tangent.y / ((float)sa + 1);
        scene->dev_passes[(TANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] +=
            tangent.z / ((float)sa + 1);

        scene->dev_passes[(BITANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] +=
            bitangent.x / ((float)sa + 1);
        scene->dev_passes[(BITANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] +=
            bitangent.y / ((float)sa + 1);
        scene->dev_passes[(BITANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] +=
            bitangent.z / ((float)sa + 1);

        scene->dev_samples[idx]++;
    }
    scene->dev_randstate[idx] = rnd;
}


void printExp(Exp exp) {
    switch (exp.type) {
    case Exp::Type::NUL:
        std::cout << "EXP(" << exp.idx << "): NUL";
        break;
    case Exp::Type::NUM:
        std::cout << "EXP(" << exp.idx << "): N(" << exp.n << ")";
        break;
    case Exp::Type::VAR:
        std::cout << "EXP(" << exp.idx << "): VAR(" << exp.x << ")";
        break;
    case Exp::Type::VEC:
        std::cout << "EXP(" << exp.idx << "): VEC(";
        printExp(*exp.e1);
        std::cout << ", ";
        printExp(*exp.e2);
        std::cout << ", ";
        printExp(*exp.e3);
        std::cout << ")";
        break;
    case Exp::Type::SUM:
        std::cout << "EXP(" << exp.idx << "): SUM(";
        printExp(*exp.e1);
        std::cout << " + ";
        printExp(*exp.e2);
        std::cout << ")";
        break;
    }
}

void printStatement(Statement sta) {
    switch (sta.type) {
    case Statement::Type::NUL:
        std::cout << "STATEMENT: NUL";
        break;
    case Statement::Type::SKIP:
        std::cout << "STATEMENT: SKIP";
        break;
    case Statement::Type::SEQ:
        std::cout << "STATEMENT: SEQ (";
        printStatement(*sta.s1);
        std::cout << ", ";
        printStatement(*sta.s2);
        std::cout << ")";
        break;
    case Statement::Type::ASS:
        std::cout << "STATEMENT: ASS(" << sta.x << " = ";
        printExp(*sta.e);
        std::cout << ")";
        break;
    case Statement::Type::IF:
        std::cout << "STATEMENT: IF (";
        printStatement(*sta.s1);
        std::cout << ", ";
        printStatement(*sta.s2);
        std::cout << ")";
        break;
    }
}



int renderSetup(sycl::queue& q, Scene* scene, dev_Scene* dev_scene) {

    BOOST_LOG_TRIVIAL(info) << "Initializing rendering";

    dev_Scene* temp = new dev_Scene(scene);

    copy_scene(temp, dev_scene, q);
    
    BOOST_LOG_TRIVIAL(info) << "OSL MATERIAL TEST";

    OslMaterial* osl = new OslMaterial();

    osl->program = generate_statement();

    BOOST_LOG_TRIVIAL(info) << "PARSED";

    printStatement(*osl->program);

    OslMaterial* dev_osl = sycl::malloc_device<OslMaterial>(1, q);;

    BOOST_LOG_TRIVIAL(info) << "COPYING";

    copy_osl_material(osl, dev_osl, q);

    BOOST_LOG_TRIVIAL(info) << "COPIED";

    int* c = sycl::malloc_device<int>(1, q);

    q.submit([&](cl::sycl::handler& h) {
        sycl::stream out = sycl::stream(2048, 1024, h);
        h.parallel_for(sycl::range(100000),
            [=](sycl::id<1> i) {
                thread_count_kernel(c, i, out);
            });
        }).wait();



    BOOST_LOG_TRIVIAL(debug) << "Starting setup kernels";

    q.submit([&](cl::sycl::handler& h) {
        sycl::stream out = sycl::stream(4096, 1024, h);
        h.parallel_for(sycl::range(scene->camera.xRes * scene->camera.yRes),
            [=](sycl::id<1> i) {
                setupKernel(dev_scene, i, out, dev_osl);
            });
    }).wait();

    BOOST_LOG_TRIVIAL(info) << "Setup finished";

    //TODO: figure out how to manage max samples (noise cutoff?)


    sycl::range global{ 1920,1080 };
    sycl::range local{8,8};

    for (int s = 0; s < 1; s++) {
        q.submit([&](cl::sycl::handler& h) {
            h.parallel_for(sycl::nd_range{global, local},
                [=](sycl::nd_item<2> it) {
                    renderingKernel(dev_scene, it.get_global_id(0) * 1080 + it.get_global_id(1), s);
                });
            });
    }

    BOOST_LOG_TRIVIAL(info) << "All samples added to the queue";

    return 0;
}

