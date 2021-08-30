/**
 * Library to interact with an HM-10 Bluetooth Module.
 */
#include "main.h"

#define MAX_STEPS 100
//#define BUDGET 9  // Energy Budget in mW. This is Total Budget / (sample rate * total budget * seq_length)
#define MILLI_FACTOR 1000
#define SAMPLE_RATE 2  // Sample period (1 / freq)

// Variable to track ADC conversion
volatile int16_t nAdc;


// Allocate memory for input features
static matrix inputFeatures, outputLabels;

// test input image
#pragma PERSISTENT(array)
static dtype array[3072] = {943, 927, 931, 931, 931, 931, 931, 931, 931, 931, 935, 935, 935, 935, 935, 935, 935, 931, 927, 923, 931, 931, 931, 935, 931, 935, 931, 931, 931, 935, 935, 931, 955, 943, 943, 943, 943, 943, 943, 943, 943, 943, 947, 947, 947, 947, 947, 947, 951, 947, 947, 939, 939, 939, 943, 947, 947, 947, 943, 943, 943, 947, 947, 943, 951, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 943, 943, 947, 947, 943, 939, 911, 927, 927, 939, 939, 939, 939, 939, 939, 943, 943, 939, 955, 943, 943, 943, 943, 943, 943, 943, 939, 939, 939, 939, 939, 939, 943, 943, 935, 931, 915, 895, 746, 839, 831, 915, 947, 939, 939, 939, 939, 943, 943, 943, 951, 939, 943, 943, 943, 943, 943, 943, 939, 939, 943, 943, 939, 939, 943, 943, 947, 935, 879, 815, 654, 783, 859, 923, 951, 943, 943, 943, 943, 947, 947, 947, 959, 947, 947, 947, 947, 947, 943, 943, 939, 943, 951, 951, 939, 931, 943, 919, 835, 779, 742, 698, 662, 738, 831, 907, 947, 947, 947, 947, 947, 951, 951, 951, 915, 915, 931, 927, 939, 951, 951, 947, 951, 951, 959, 959, 903, 899, 935, 887, 734, 646, 638, 618, 578, 574, 626, 795, 935, 947, 943, 943, 943, 947, 951, 959, 851, 899, 923, 911, 919, 939, 951, 955, 959, 959, 959, 963, 807, 879, 935, 859, 775, 742, 738, 694, 662, 638, 650, 746, 919, 939, 935, 935, 939, 947, 951, 955, 867, 887, 903, 903, 911, 927, 947, 955, 955, 955, 951, 959, 791, 883, 935, 923, 839, 839, 879, 835, 839, 843, 871, 875, 903, 915, 915, 923, 923, 943, 951, 955, 473, 477, 497, 546, 690, 903, 943, 951, 947, 943, 943, 935, 859, 907, 931, 947, 915, 911, 927, 903, 903, 871, 807, 742, 690, 670, 670, 746, 895, 943, 947, 955, 437, 413, 433, 445, 586, 891, 911, 919, 947, 939, 927, 923, 919, 927, 931, 923, 927, 927, 919, 895, 766, 658, 586, 550, 538, 514, 485, 598, 867, 939, 943, 951, 783, 754, 799, 803, 839, 895, 855, 847, 867, 883, 879, 843, 839, 847, 867, 883, 903, 907, 903, 875, 734, 702, 726, 714, 746, 682, 570, 742, 879, 927, 939, 947, 775, 766, 811, 859, 895, 859, 815, 686, 710, 831, 698, 393, 373, 405, 445, 489, 550, 614, 811, 895, 875, 883, 895, 871, 887, 851, 787, 891, 879, 887, 931, 943, 453, 445, 453, 501, 554, 682, 766, 762, 835, 867, 634, 216, 180, 196, 212, 265, 409, 638, 887, 939, 935, 911, 895, 831, 811, 847, 851, 799, 718, 754, 847, 887, 244, 277, 252, 273, 493, 558, 606, 783, 859, 827, 654, 413, 381, 405, 554, 726, 831, 887, 879, 823, 734, 634, 590, 526, 501, 522, 546, 534, 514, 554, 730, 791, 160, 232, 341, 509, 530, 385, 477, 654, 694, 738, 730, 726, 734, 795, 875, 803, 698, 638, 582, 530, 465, 393, 377, 397, 421, 429, 489, 554, 602, 630, 754, 742, 52, 104, 538, 827, 554, 473, 566, 690, 726, 831, 883, 915, 899, 923, 907, 706, 578, 554, 570, 582, 618, 598, 598, 618, 630, 642, 694, 750, 762, 714, 662, 630, 20, 232, 803, 903, 791, 799, 851, 907, 919, 935, 931, 923, 839, 895, 887, 843, 795, 722, 775, 754, 758, 779, 771, 738, 690, 686, 646, 578, 546, 526, 514, 554, 156, 582, 762, 746, 738, 771, 779, 779, 779, 766, 771, 762, 710, 722, 618, 590, 582, 626, 586, 453, 457, 530, 505, 445, 369, 365, 373, 377, 421, 485, 518, 518, 489, 650, 574, 550, 526, 514, 509, 522, 526, 514, 509, 518, 518, 497, 417, 401, 409, 473, 449, 377, 377, 377, 349, 333, 321, 333, 373, 405, 433, 461, 485, 522, 293, 305, 309, 321, 337, 349, 349, 361, 377, 409, 429, 453, 461, 473, 473, 481, 461, 441, 425, 401, 381, 341, 317, 321, 321, 309, 321, 329, 369, 453, 501, 546, 52, 12, 36, 72, 72, 84, 80, 88, 104, 136, 168, 192, 208, 240, 265, 281, 285, 289, 269, 240, 220, 212, 212, 228, 228, 228, 289, 349, 417, 481, 522, 550, 144, 44, 32, 128, 144, 88, 32, 12, 4, 0, 0, 0, 24, 20, 4, 12, 52, 96, 84, 84, 84, 88, 120, 156, 228, 341, 453, 493, 465, 489, 538, 614, 140, 104, 52, 108, 285, 281, 196, 108, 60, 20, 8, 0, 68, 228, 124, 40, 16, 16, 28, 56, 100, 164, 248, 345, 489, 578, 530, 457, 469, 530, 586, 690, 64, 52, 16, 12, 180, 261, 216, 144, 72, 16, 8, 0, 28, 473, 646, 526, 449, 421, 421, 437, 473, 554, 618, 606, 509, 421, 425, 481, 518, 570, 658, 738, 160, 48, 0, 0, 48, 120, 128, 84, 28, 8, 8, 12, 0, 273, 730, 823, 787, 779, 783, 750, 690, 602, 493, 413, 381, 417, 489, 518, 530, 610, 686, 742, 277, 104, 4, 4, 16, 48, 72, 48, 16, 8, 8, 16, 4, 128, 614, 815, 783, 766, 718, 622, 477, 365, 325, 377, 469, 501, 501, 518, 578, 650, 694, 738, 333, 188, 4, 8, 8, 20, 28, 16, 4, 4, 4, 12, 4, 108, 570, 823, 795, 678, 485, 341, 297, 341, 409, 485, 514, 489, 485, 530, 590, 662, 706, 746, 369, 216, 24, 12, 8, 4, 4, 4, 4, 4, 4, 4, 0, 60, 409, 630, 469, 297, 224, 297, 397, 461, 489, 497, 493, 501, 514, 546, 594, 650, 710, 754, 349, 172, 76, 44, 32, 20, 8, 8, 12, 12, 12, 8, 0, 16, 168, 285, 212, 228, 321, 453, 530, 538, 493, 465, 481, 526, 558, 574, 626, 678, 730, 754, 329, 184, 144, 124, 108, 88, 68, 64, 72, 76, 80, 76, 76, 92, 148, 257, 349, 417, 465, 514, 558, 526, 469, 461, 493, 526, 558, 594, 638, 698, 742, 750, 341, 248, 232, 220, 204, 188, 184, 192, 196, 204, 212, 220, 236, 273, 325, 417, 465, 509, 534, 509, 509, 473, 457, 489, 518, 546, 566, 598, 634, 674, 722, 746, 943, 927, 931, 931, 931, 931, 931, 931, 931, 931, 935, 935, 935, 935, 935, 931, 927, 927, 935, 935, 931, 927, 931, 935, 935, 935, 931, 931, 931, 935, 935, 931, 955, 943, 943, 943, 943, 943, 943, 943, 943, 943, 947, 947, 947, 947, 947, 947, 939, 939, 947, 947, 943, 939, 947, 947, 947, 947, 943, 943, 943, 947, 947, 943, 951, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 943, 939, 935, 939, 943, 943, 923, 943, 935, 939, 939, 939, 939, 939, 939, 943, 943, 939, 955, 943, 943, 943, 943, 943, 943, 943, 939, 939, 939, 939, 939, 939, 943, 943, 935, 931, 923, 907, 771, 867, 843, 915, 943, 939, 939, 939, 939, 943, 943, 943, 951, 939, 943, 943, 943, 943, 943, 943, 939, 939, 943, 943, 939, 939, 943, 943, 955, 951, 903, 843, 690, 823, 875, 919, 943, 943, 943, 943, 947, 947, 947, 947, 959, 943, 943, 943, 943, 943, 947, 943, 939, 943, 947, 947, 943, 935, 951, 927, 867, 823, 795, 754, 718, 787, 863, 915, 943, 947, 947, 947, 947, 951, 951, 951, 919, 911, 923, 915, 931, 947, 951, 951, 943, 943, 947, 951, 919, 915, 951, 907, 791, 722, 722, 706, 654, 638, 678, 827, 955, 951, 947, 943, 947, 955, 951, 951, 883, 923, 939, 931, 939, 951, 955, 951, 951, 951, 947, 955, 819, 891, 947, 875, 819, 807, 807, 766, 730, 698, 706, 799, 959, 959, 955, 955, 959, 959, 959, 955, 939, 947, 955, 959, 963, 955, 951, 947, 947, 947, 951, 959, 795, 887, 939, 927, 855, 867, 915, 875, 887, 899, 935, 943, 963, 955, 959, 963, 963, 963, 963, 955, 562, 554, 570, 622, 754, 939, 947, 939, 935, 943, 951, 951, 867, 915, 939, 951, 923, 923, 947, 931, 951, 935, 879, 819, 758, 718, 722, 799, 943, 967, 963, 963, 522, 485, 501, 509, 638, 919, 915, 907, 931, 939, 947, 951, 939, 943, 951, 943, 947, 951, 951, 931, 827, 738, 662, 626, 598, 562, 534, 650, 915, 967, 963, 963, 851, 811, 847, 847, 871, 911, 855, 839, 855, 891, 907, 887, 879, 887, 903, 919, 939, 947, 951, 927, 819, 795, 803, 779, 791, 714, 606, 783, 923, 963, 967, 963, 831, 811, 847, 871, 903, 879, 835, 698, 722, 855, 738, 449, 457, 485, 518, 554, 610, 670, 867, 947, 931, 935, 939, 907, 915, 879, 815, 923, 911, 923, 959, 967, 522, 501, 501, 526, 582, 730, 807, 799, 879, 923, 690, 285, 281, 293, 293, 337, 457, 674, 911, 959, 951, 927, 915, 847, 835, 875, 879, 827, 746, 791, 887, 927, 325, 345, 317, 341, 566, 622, 630, 803, 915, 895, 722, 485, 449, 469, 606, 771, 851, 891, 879, 815, 746, 666, 618, 554, 534, 558, 586, 570, 550, 614, 791, 851, 212, 281, 393, 578, 606, 429, 461, 634, 722, 779, 779, 775, 779, 839, 915, 843, 726, 662, 602, 546, 501, 445, 425, 445, 473, 485, 542, 606, 658, 698, 827, 815, 60, 116, 562, 867, 602, 493, 534, 650, 726, 839, 899, 939, 939, 967, 955, 758, 638, 618, 634, 654, 686, 662, 662, 686, 698, 710, 762, 819, 831, 787, 734, 702, 20, 248, 831, 931, 823, 831, 851, 899, 923, 947, 955, 955, 887, 955, 955, 915, 871, 803, 867, 855, 851, 859, 851, 819, 775, 766, 726, 662, 626, 586, 574, 618, 180, 622, 819, 787, 791, 847, 847, 835, 827, 815, 831, 831, 775, 795, 706, 678, 646, 686, 654, 534, 550, 630, 602, 542, 461, 449, 457, 465, 501, 534, 566, 570, 542, 718, 642, 618, 610, 610, 602, 602, 602, 590, 590, 598, 598, 582, 505, 489, 481, 538, 514, 437, 449, 469, 449, 413, 389, 413, 445, 469, 485, 501, 534, 578, 349, 361, 361, 373, 393, 409, 409, 421, 445, 477, 497, 526, 550, 546, 530, 534, 546, 534, 509, 477, 437, 405, 389, 369, 377, 401, 401, 393, 417, 477, 542, 598, 100, 44, 64, 104, 104, 100, 100, 120, 144, 172, 204, 236, 277, 301, 309, 317, 349, 353, 325, 289, 269, 273, 277, 277, 285, 313, 357, 401, 453, 497, 546, 586, 184, 64, 52, 176, 180, 100, 44, 32, 16, 8, 8, 16, 52, 72, 76, 92, 116, 152, 132, 124, 152, 176, 200, 232, 281, 361, 461, 493, 461, 493, 558, 642, 164, 108, 76, 164, 325, 281, 200, 124, 60, 20, 8, 0, 68, 257, 200, 144, 120, 120, 120, 140, 172, 220, 285, 389, 497, 526, 481, 421, 445, 538, 610, 718, 60, 40, 40, 48, 176, 208, 172, 132, 72, 16, 8, 4, 32, 469, 634, 514, 449, 421, 413, 421, 429, 461, 505, 505, 425, 345, 377, 465, 522, 590, 690, 779, 160, 40, 12, 16, 24, 48, 48, 40, 24, 4, 4, 8, 0, 232, 514, 522, 509, 493, 477, 453, 441, 385, 301, 265, 285, 373, 473, 530, 566, 650, 730, 791, 309, 116, 4, 4, 4, 8, 12, 8, 4, 0, 0, 0, 4, 48, 180, 188, 184, 192, 200, 196, 168, 152, 192, 309, 441, 505, 514, 542, 614, 706, 750, 795, 377, 208, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 8, 12, 100, 128, 100, 100, 100, 116, 164, 265, 369, 453, 497, 505, 509, 558, 630, 718, 766, 807, 409, 240, 28, 8, 8, 12, 12, 8, 4, 0, 0, 4, 12, 4, 76, 124, 68, 52, 108, 232, 361, 461, 505, 497, 493, 522, 542, 582, 638, 706, 771, 811, 397, 204, 92, 48, 40, 44, 40, 28, 16, 16, 16, 12, 24, 20, 52, 84, 108, 200, 309, 393, 453, 505, 505, 501, 514, 554, 594, 618, 674, 738, 791, 811, 385, 228, 176, 140, 120, 112, 104, 92, 84, 84, 88, 92, 108, 124, 160, 220, 281, 353, 409, 449, 485, 489, 489, 509, 534, 558, 598, 642, 690, 758, 803, 811, 405, 301, 269, 244, 224, 212, 212, 220, 220, 224, 232, 248, 269, 285, 337, 385, 413, 437, 465, 485, 509, 497, 501, 526, 546, 582, 610, 650, 686, 734, 783, 803, 943, 927, 931, 931, 931, 931, 931, 931, 931, 931, 935, 935, 935, 935, 935, 935, 935, 935, 935, 931, 939, 939, 931, 923, 927, 935, 931, 931, 931, 935, 935, 931, 955, 943, 943, 943, 943, 943, 943, 943, 943, 943, 947, 947, 947, 947, 947, 947, 935, 935, 939, 939, 951, 955, 951, 943, 939, 947, 943, 943, 943, 947, 947, 943, 951, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 939, 943, 939, 927, 927, 939, 947, 935, 955, 943, 939, 939, 939, 939, 939, 939, 943, 943, 939, 955, 943, 943, 943, 943, 943, 943, 943, 939, 939, 939, 939, 939, 939, 943, 939, 923, 927, 931, 927, 791, 879, 855, 923, 943, 939, 939, 939, 939, 943, 943, 943, 951, 939, 943, 943, 943, 943, 943, 943, 939, 939, 943, 943, 939, 939, 943, 943, 947, 951, 923, 879, 718, 835, 887, 931, 951, 943, 943, 943, 947, 947, 947, 947, 955, 943, 943, 943, 943, 943, 943, 943, 939, 943, 947, 947, 947, 939, 951, 931, 875, 843, 831, 803, 758, 811, 883, 931, 951, 943, 943, 943, 947, 951, 951, 951, 919, 915, 927, 923, 935, 947, 943, 943, 947, 947, 951, 955, 923, 919, 955, 915, 819, 762, 766, 762, 710, 686, 710, 847, 959, 939, 935, 943, 947, 951, 951, 955, 891, 935, 955, 939, 939, 947, 943, 947, 955, 955, 951, 959, 815, 887, 943, 875, 843, 843, 847, 815, 787, 750, 742, 819, 963, 955, 951, 955, 955, 955, 955, 955, 967, 975, 987, 975, 963, 951, 943, 943, 951, 951, 951, 959, 787, 875, 927, 919, 871, 891, 943, 911, 939, 943, 963, 967, 975, 963, 963, 963, 959, 959, 959, 955, 598, 594, 614, 646, 766, 935, 935, 931, 939, 943, 947, 943, 859, 907, 931, 947, 931, 943, 967, 959, 991, 975, 907, 847, 783, 746, 742, 807, 943, 959, 959, 959, 566, 534, 550, 550, 662, 927, 903, 899, 935, 939, 939, 943, 943, 947, 955, 947, 955, 963, 967, 955, 855, 766, 690, 654, 638, 614, 574, 666, 919, 959, 955, 959, 899, 863, 899, 895, 911, 927, 847, 827, 859, 891, 903, 879, 895, 903, 923, 935, 951, 959, 967, 951, 835, 815, 831, 811, 847, 787, 658, 811, 935, 955, 959, 959, 891, 871, 899, 939, 967, 911, 835, 698, 734, 859, 754, 485, 505, 530, 558, 590, 646, 698, 883, 951, 943, 955, 963, 935, 951, 919, 851, 951, 939, 935, 971, 971, 610, 590, 566, 606, 662, 775, 823, 819, 907, 939, 734, 369, 365, 365, 361, 393, 518, 718, 935, 967, 967, 951, 935, 871, 851, 883, 895, 859, 787, 823, 911, 939, 433, 457, 401, 409, 622, 658, 658, 831, 939, 915, 762, 554, 526, 542, 674, 831, 895, 931, 911, 851, 783, 698, 654, 590, 562, 578, 610, 606, 590, 642, 815, 867, 309, 377, 465, 614, 626, 441, 473, 646, 730, 791, 795, 803, 811, 871, 947, 871, 746, 690, 638, 598, 554, 493, 473, 493, 514, 522, 582, 646, 698, 738, 855, 835, 140, 188, 606, 883, 602, 493, 538, 650, 722, 847, 903, 935, 931, 963, 955, 762, 654, 650, 682, 710, 750, 730, 730, 750, 758, 766, 819, 871, 875, 835, 775, 734, 96, 317, 871, 959, 851, 847, 875, 919, 951, 987, 983, 959, 883, 959, 967, 939, 915, 859, 923, 919, 927, 939, 931, 899, 851, 839, 791, 718, 678, 646, 634, 662, 285, 718, 891, 867, 871, 919, 923, 911, 911, 915, 915, 887, 831, 863, 775, 754, 738, 783, 746, 626, 646, 722, 694, 634, 554, 542, 534, 526, 562, 606, 634, 626, 646, 831, 779, 758, 750, 762, 771, 775, 771, 762, 758, 758, 754, 746, 654, 618, 618, 682, 654, 582, 594, 614, 578, 546, 522, 538, 558, 566, 578, 586, 594, 626, 437, 453, 489, 509, 538, 570, 590, 602, 610, 642, 662, 690, 726, 746, 722, 702, 690, 674, 654, 622, 594, 558, 530, 509, 518, 534, 518, 489, 505, 554, 586, 626, 164, 100, 140, 192, 208, 224, 232, 244, 248, 281, 309, 349, 425, 485, 505, 505, 509, 505, 481, 449, 425, 417, 413, 409, 421, 441, 461, 477, 514, 546, 566, 598, 220, 80, 76, 212, 232, 164, 120, 96, 68, 60, 60, 80, 168, 224, 240, 248, 285, 325, 309, 305, 313, 317, 333, 361, 405, 473, 554, 554, 501, 514, 550, 634, 180, 104, 72, 164, 337, 305, 228, 148, 84, 44, 28, 28, 140, 365, 313, 248, 240, 248, 252, 277, 297, 333, 397, 493, 586, 598, 542, 457, 465, 534, 586, 702, 68, 36, 32, 44, 184, 228, 188, 140, 80, 28, 16, 12, 60, 538, 718, 594, 526, 501, 497, 509, 505, 534, 578, 566, 465, 365, 389, 465, 518, 578, 662, 762, 140, 28, 12, 16, 28, 68, 68, 48, 28, 12, 8, 12, 8, 257, 586, 594, 578, 566, 550, 518, 489, 425, 333, 277, 281, 353, 453, 505, 542, 634, 706, 779, 257, 84, 4, 8, 0, 20, 36, 20, 8, 0, 0, 4, 4, 44, 236, 273, 269, 277, 269, 236, 196, 168, 184, 285, 409, 465, 481, 514, 590, 686, 734, 787, 329, 172, 4, 8, 0, 8, 20, 8, 0, 0, 0, 0, 0, 8, 152, 216, 184, 172, 144, 136, 156, 224, 329, 421, 461, 461, 473, 526, 602, 698, 750, 799, 373, 200, 12, 4, 0, 4, 12, 8, 4, 0, 0, 4, 8, 0, 112, 188, 92, 48, 88, 220, 325, 397, 445, 449, 453, 477, 505, 550, 606, 686, 754, 807, 357, 148, 44, 16, 8, 16, 16, 8, 4, 4, 4, 8, 24, 8, 52, 96, 100, 164, 248, 329, 405, 453, 449, 445, 461, 505, 550, 582, 646, 718, 775, 807, 329, 144, 88, 68, 60, 60, 52, 48, 48, 52, 56, 60, 80, 84, 108, 180, 269, 325, 341, 353, 421, 441, 429, 449, 477, 509, 554, 606, 658, 734, 787, 803, 333, 192, 152, 148, 140, 132, 136, 152, 160, 164, 176, 184, 180, 192, 236, 297, 333, 369, 389, 389, 429, 425, 433, 469, 493, 534, 566, 614, 654, 714, 766, 799};
static dtype input_buffer3[100] = {0};

#pragma PERSISTENT(filename0)
static char filename0[16] = "layer0.txt";

#pragma PERSISTENT(filename1)
static char filename1[16] = "layer1.txt";

#pragma PERSISTENT(filename2)
static char filename2[16] = "layer2.txt";

#pragma PERSISTENT(filename3)
static char filename3[16] = "layer3.txt";

#pragma PERSISTENT(filename4)
static char filename4[16] = "layer4.txt";

#pragma PERSISTENT(filename5)
static char filename5[16] = "layer5.txt";

#pragma PERSISTENT(filename6)
static char filename6[16] = "layer6.txt";

#pragma PERSISTENT(filename7)
static char filename7[16] = "layer7.txt";

#pragma PERSISTENT(filename8)
static char filename8[16] = "layer8_1.txt";

#pragma PERSISTENT(filename9)
static char filename9[16] = "layer9.txt";

int16_t label;

/*
 * main.c
 */
void main(void){

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
//
//    // Initialize GPIO System
    init_gpio();
//
//    // Initialize the clock and baudrate
    init_clock_system();

    uint16_t pt = 0;

    MODEL_ARRAY[0] = 0;
    pt ++;

    readSD_data16(MODEL_ARRAY + pt, filename0, 458);

    pt += 458;

    readSD_data16(MODEL_ARRAY + pt, filename1, 2330);

    pt += 2330;

    readSD_data16(MODEL_ARRAY + pt, filename2, 6);

    pt += 6;

    readSD_data16(MODEL_ARRAY + pt, filename3, 2330);

    pt += 2330;

    readSD_data16(MODEL_ARRAY + pt, filename4, 2330);

    pt += 2330;

    readSD_data16(MODEL_ARRAY + pt, filename5, 6);

    pt += 6;

    readSD_data16(MODEL_ARRAY + pt, filename6, 1);

    pt ++;

    readSD_data16(MODEL_ARRAY + pt, filename7, 1);

    pt ++;

    readSD_data16(MODEL_ARRAY + pt, filename8, 16406);

    pt += 16406;

    readSD_data16(MODEL_ARRAY + pt, filename9, 176);

    inputFeatures.numRows = 32;
    inputFeatures.numCols = 32;
    inputFeatures.data = array;

    outputLabels.numRows = 10;
    outputLabels.numCols = 2;
    outputLabels.data = input_buffer3;

    apply_model(&outputLabels, &inputFeatures);
    label = argmax(&outputLabels);

//    writeSD_data16(layer_0, filename0, 458);
//
//    writeSD_data16(layer_1, filename1, 2330);
//
//    writeSD_data16(layer_2, filename2, 6);
//
//    writeSD_data16(layer_3, filename3, 2330);
//
//    writeSD_data16(layer_4, filename4, 2330);
//
//    writeSD_data16(layer_5, filename5, 6);
//
//    writeSD_data16(layer_6, filename6, 1);
//
//    writeSD_data16(layer_7, filename7, 1);
//
//    writeSD_data16(layer_8, filename8, 16406);
//
//    writeSD_data16(layer_9, filename9, 176);

    __no_operation();

}

