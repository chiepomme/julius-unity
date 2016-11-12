#include "lib.h"

static Jconf* jconf = NULL;
static Recog* recog = NULL;

static debug_log_func_type debug_log = NULL;
static audio_read_callback_func_type audio_read_callback = NULL;
static result_func_type output_result = NULL;

EXPORT_API void set_audio_callback(audio_read_callback_func_type callback)
{
	audio_read_callback = callback;
}

EXPORT_API void set_debug_log_func(debug_log_func_type debug_log_func)
{
	debug_log = debug_log_func;
}

EXPORT_API void set_result_func(result_func_type result_func)
{
	output_result = result_func;
}

EXPORT_API void set_log_to_file(const char* path)
{
	jlog_set_output(fopen(path, "w"));
}

EXPORT_API void set_log_to_stdout(bool use_stderr_instead)
{
	if (use_stderr_instead)
	{
		jlog_set_output(stderr);
	}
	else
	{
		jlog_set_output(stdout);
	}
}

EXPORT_API void start(char* jconf_path)
{
	if (audio_read_callback == NULL)
	{
		debug_log("audio callback is null, so use default audio in\n");
	}

	if (output_result == NULL)
	{
		debug_log("output_result is null\n");
		return;
	}

	clean_up_if_exists();

	jconf = j_config_load_file_new(jconf_path);
	if (jconf == NULL)
	{
		debug_log("failed to load jconf\n");
		return;
	}

	recog = j_recog_new();
	if (recog == NULL)
	{
		debug_log("failed to create recog\n");
		return;
	}

	if (create_engine())
	{
		debug_log("successfully engine created");
	}
	else
	{
		debug_log("failed to create engine");
		return;
	}

	if (j_recognize_stream(recog) < 0)
	{
		debug_log("failed to open recognize stream");
		return;
	}

	clean_up_if_exists();
}

EXPORT_API void finish()
{
	if (recog != NULL)
	{
		j_close_stream(recog);
	}
}

void clean_up_if_exists()
{
	if (recog != NULL)
	{
		// jconf will be released inside this
		j_recog_free(recog);
		jconf = NULL;
		recog = NULL;
	}

	if (jconf != NULL) {
		j_jconf_free(NULL);
		jconf = NULL;
	}
}

static void on_process_online(Recog *recog, void *dummy)
{
	jlog("on_process_online\n");
	debug_log("on_process_online");
}

static void on_process_offline(Recog *recog, void *dummy)
{
	jlog("on_process_offline\n");
	debug_log("on_process_offline");
}

static void on_speech_ready(Recog *recog, void *dummy)
{
	jlog("on_speech_ready\n");
	debug_log("on_speech_ready");
}

static void on_speech_start(Recog *recog, void *dummy)
{
	jlog("on_speech_start\n");
	debug_log("on_speech_start");
}

static void on_speech_stop(Recog *recog, void *dummy)
{
	jlog("on_speech_stop\n");
	debug_log("on_speech_stop");
}

static void on_recognition_begin(Recog *recog, void *dummy)
{
	jlog("on_recognition_begin\n");
	debug_log("on_recognition_begin");
}

static void on_recognition_end(Recog *recog, void *dummy)
{
	jlog("on_recognition_end\n");
	debug_log("on_recognition_end");
}

static void put_hypo_phoneme(WORD_ID *seq, int n, WORD_INFO *winfo)
{
	int i, j;
	WORD_ID w;
	static char buf[MAX_HMMNAME_LEN];

	if (seq != NULL) {
		for (i = 0; i < n; i++) {
			if (i > 0) jlog(" |");
			w = seq[i];
			for (j = 0; j < winfo->wlen[w]; j++) {
				center_name(winfo->wseq[w][j]->name, buf);
				jlog(" %s", buf);
			}
		}
	}
	jlog("\n");
}

static void on_pass1_frame(Recog *recog, void *dummy)
{
	jlog("on_pass1_frame\n");
	debug_log("on_pass1_frame");

	int i, j;
	int len;
	WORD_INFO *winfo;
	WORD_ID *seq;
	int seqnum;
	int n = 0;
	Sentence *s;
	RecogProcess *r;
	HMM_Logical *p;
	SentenceAlign *align;

	char result_string[65535] = "";
	char temp_buffer[65535];

	/* all recognition results are stored at each recognition process
	instance */
	for (r = recog->process_list; r; r = r->next) {

		/* skip the process if the process is not alive */
		if (!r->live) continue;

		/* result are in r->result.  See recog.h for details */

		/* check result status */
		if (r->result.status < 0) {      /* no results obtained */
										 /* outout message according to the status code */
			switch (r->result.status) {
			case J_RESULT_STATUS_REJECT_POWER:
				debug_log("<input rejected by power>\n");
				break;
			case J_RESULT_STATUS_TERMINATE:
				debug_log("<input teminated by request>\n");
				break;
			case J_RESULT_STATUS_ONLY_SILENCE:
				debug_log("<input rejected by decoder (silence input result)>\n");
				break;
			case J_RESULT_STATUS_REJECT_GMM:
				debug_log("<input rejected by GMM>\n");
				break;
			case J_RESULT_STATUS_REJECT_SHORT:
				debug_log("<input rejected by short input>\n");
				break;
			case J_RESULT_STATUS_REJECT_LONG:
				debug_log("<input rejected by long input>\n");
				break;
			case J_RESULT_STATUS_FAIL:
				debug_log("<search failed>\n");
				break;
			}
			/* continue to next process instance */
			continue;
		}

		winfo = r->lm->winfo;

		s = &(r->result.pass1);
		seq = s->word;
		seqnum = s->word_num;

		/* output word sequence like Julius */
		for (i = 0; i < seqnum; i++) jlog(" %s", winfo->woutput[seq[i]]);
		strncat(result_string, "sentence,", sizeof(result_string) - 1);

		for (i = 0; i < seqnum; i++)
		{
			strncat(result_string, winfo->woutput[seq[i]], sizeof(result_string) - 1);
			if (i + 1 == seqnum)
			{
				strncat(result_string, "\n", sizeof(result_string) - 1);
			}
			else
			{
				strncat(result_string, ",", sizeof(result_string) - 1);
			}
		}

		jlog("\n");
		/* LM entry sequence */
		jlog("wseq%d:", n + 1);
		for (i = 0; i < seqnum; i++) jlog(" %s", winfo->wname[seq[i]]);

		strncat(result_string, "words,", sizeof(result_string) - 1);
		for (i = 0; i < seqnum; i++)
		{
			strncat(result_string, winfo->wname[seq[i]], sizeof(result_string) - 1);
			if (i + 1 == seqnum)
			{
				strncat(result_string, "\n", sizeof(result_string) - 1);
			}
			else
			{
				strncat(result_string, ",", sizeof(result_string) - 1);
			}
		}

		jlog("\n");
		/* phoneme sequence */
		jlog("phseq%d:", n + 1);
		put_hypo_phoneme(seq, seqnum, winfo);
		jlog("\n");

		jlog("\n");
		/* AM and LM scores */
		jlog("score%d: %f", n + 1, s->score);
		if (r->lmtype == LM_PROB) { /* if this process uses N-gram */
			jlog(" (AM: %f  LM: %f)", s->score_am, s->score_lm);
		}

		strncat(result_string, "total_score,", sizeof(result_string) - 1);
		snprintf(temp_buffer, sizeof(temp_buffer), "%f", s->score);
		strncat(result_string, temp_buffer, sizeof(result_string) - 1);
		strncat(result_string, "\n", sizeof(result_string) - 1);

		strncat(result_string, "acoustic_model_score,", sizeof(result_string) - 1);
		snprintf(temp_buffer, sizeof(temp_buffer), "%f", s->score_am);
		strncat(result_string, temp_buffer, sizeof(result_string) - 1);
		strncat(result_string, "\n", sizeof(result_string) - 1);

		strncat(result_string, "language_model_score,", sizeof(result_string) - 1);
		snprintf(temp_buffer, sizeof(temp_buffer), "%f", s->score_lm);
		strncat(result_string, temp_buffer, sizeof(result_string) - 1);
		strncat(result_string, "\n", sizeof(result_string) - 1);
	}

	output_result(result_string, (int)strlen(result_string));
	jlog_flush();
}

static void on_pause(Recog *recog, void *dummy)
{
	jlog("on_pause\n");
	debug_log("on_pause");
}

static void on_resume(Recog *recog, void *dummy)
{
	jlog("on_resume\n");
	debug_log("on_resume");
}

static void on_pause_function(Recog *recog, void *dummy)
{
	jlog("on_pause_function\n");
	debug_log("on_pause_function");
}



static void on_result(Recog *recog, void *dummy)
{
	jlog("on_result");
	debug_log("on_result");
	write_result(recog, dummy);
}

static void write_result(Recog *recog, void *dummy)
{
	int i, j;
	int len;
	WORD_INFO *winfo;
	WORD_ID *seq;
	int seqnum;
	int n;
	Sentence *s;
	RecogProcess *r;
	HMM_Logical *p;
	SentenceAlign *align;

	char result_string[65535] = "";
	char temp_buffer[65535];

	/* all recognition results are stored at each recognition process
	instance */
	for (r = recog->process_list; r; r = r->next) {

		/* skip the process if the process is not alive */
		if (!r->live) continue;

		/* result are in r->result.  See recog.h for details */

		/* check result status */
		if (r->result.status < 0) {      /* no results obtained */
										 /* outout message according to the status code */
			switch (r->result.status) {
			case J_RESULT_STATUS_REJECT_POWER:
				debug_log("<input rejected by power>\n");
				break;
			case J_RESULT_STATUS_TERMINATE:
				debug_log("<input teminated by request>\n");
				break;
			case J_RESULT_STATUS_ONLY_SILENCE:
				debug_log("<input rejected by decoder (silence input result)>\n");
				break;
			case J_RESULT_STATUS_REJECT_GMM:
				debug_log("<input rejected by GMM>\n");
				break;
			case J_RESULT_STATUS_REJECT_SHORT:
				debug_log("<input rejected by short input>\n");
				break;
			case J_RESULT_STATUS_REJECT_LONG:
				debug_log("<input rejected by long input>\n");
				break;
			case J_RESULT_STATUS_FAIL:
				debug_log("<search failed>\n");
				break;
			}
			/* continue to next process instance */
			continue;
		}

		/* output results for all the obtained sentences */
		winfo = r->lm->winfo;

		for (n = 0; n < r->result.sentnum; n++) { /* for all sentences */

			s = &(r->result.sent[n]);
			seq = s->word;
			seqnum = s->word_num;

			/* output word sequence like Julius */
			jlog("sentence%d:", n + 1);
			for (i = 0; i < seqnum; i++) jlog(" %s", winfo->woutput[seq[i]]);
			strncat(result_string, "sentence,", sizeof(result_string) - 1);

			for (i = 0; i < seqnum; i++)
			{
				strncat(result_string, winfo->woutput[seq[i]], sizeof(result_string) - 1);
				if (i + 1 == seqnum)
				{
					strncat(result_string, "\n", sizeof(result_string) - 1);
				}
				else
				{
					strncat(result_string, ",", sizeof(result_string) - 1);
				}
			}

			jlog("\n");
			/* LM entry sequence */
			jlog("wseq%d:", n + 1);
			for (i = 0; i < seqnum; i++) jlog(" %s", winfo->wname[seq[i]]);

			strncat(result_string, "words,", sizeof(result_string) - 1);
			for (i = 0; i < seqnum; i++)
			{
				strncat(result_string, winfo->wname[seq[i]], sizeof(result_string) - 1);
				if (i + 1 == seqnum)
				{
					strncat(result_string, "\n", sizeof(result_string) - 1);
				}
				else
				{
					strncat(result_string, ",", sizeof(result_string) - 1);
				}
			}

			jlog("\n");
			/* phoneme sequence */
			jlog("phseq%d:", n + 1);
			put_hypo_phoneme(seq, seqnum, winfo);
			jlog("\n");
			/* confidence scores */
			jlog("cmscore%d:", n + 1);
			for (i = 0; i < seqnum; i++) jlog(" %5.3f", s->confidence[i]);

			strncat(result_string, "confidence_score,", sizeof(result_string) - 1);
			for (i = 0; i < seqnum; i++)
			{
				snprintf(temp_buffer, sizeof(temp_buffer), "%5.3f", s->confidence[i]);
				strncat(result_string, temp_buffer, sizeof(result_string) - 1);
				if (i + 1 == seqnum)
				{
					strncat(result_string, "\n", sizeof(result_string) - 1);
				}
				else
				{
					strncat(result_string, ",", sizeof(result_string) - 1);
				}
			}

			jlog("\n");
			/* AM and LM scores */
			jlog("score%d: %f", n + 1, s->score);
			if (r->lmtype == LM_PROB) { /* if this process uses N-gram */
				jlog(" (AM: %f  LM: %f)", s->score_am, s->score_lm);
			}

			strncat(result_string, "total_score,", sizeof(result_string) - 1);
			snprintf(temp_buffer, sizeof(temp_buffer), "%f", s->score);
			strncat(result_string, temp_buffer, sizeof(result_string) - 1);
			strncat(result_string, "\n", sizeof(result_string) - 1);

			strncat(result_string, "acoustic_model_score,", sizeof(result_string) - 1);
			snprintf(temp_buffer, sizeof(temp_buffer), "%f", s->score_am);
			strncat(result_string, temp_buffer, sizeof(result_string) - 1);
			strncat(result_string, "\n", sizeof(result_string) - 1);

			strncat(result_string, "language_model_score,", sizeof(result_string) - 1);
			snprintf(temp_buffer, sizeof(temp_buffer), "%f", s->score_lm);
			strncat(result_string, temp_buffer, sizeof(result_string) - 1);
			strncat(result_string, "\n", sizeof(result_string) - 1);

			jlog("\n");
			if (r->lmtype == LM_DFA) { /* if this process uses DFA grammar */
									   /* output which grammar the hypothesis belongs to
									   when using multiple grammars */
				if (multigram_get_all_num(r->lm) > 1) {
					jlog("grammar%d: %d\n", n + 1, s->gram_id);
				}
			}

			/* output alignment result if exist */
			for (align = s->align; align; align = align->next) {
				jlog("=== begin forced alignment ===\n");
				switch (align->unittype) {
				case PER_WORD:
					jlog("-- word alignment --\n"); break;
				case PER_PHONEME:
					jlog("-- phoneme alignment --\n"); break;
				case PER_STATE:
					jlog("-- state alignment --\n"); break;
				}
				jlog(" id: from  to    n_score    unit\n");
				jlog(" ----------------------------------------\n");
				for (i = 0; i < align->num; i++) {
					jlog("[%4d %4d]  %f  ", align->begin_frame[i], align->end_frame[i], align->avgscore[i]);
					switch (align->unittype) {
					case PER_WORD:
						jlog("%s\t[%s]\n", winfo->wname[align->w[i]], winfo->woutput[align->w[i]]);
						break;
					case PER_PHONEME:
						p = align->ph[i];
						if (p->is_pseudo) {
							jlog("{%s}\n", p->name);
						}
						else if (strmatch(p->name, p->body.defined->name)) {
							jlog("%s\n", p->name);
						}
						else {
							jlog("%s[%s]\n", p->name, p->body.defined->name);
						}
						break;
					case PER_STATE:
						p = align->ph[i];
						if (p->is_pseudo) {
							jlog("{%s}", p->name);
						}
						else if (strmatch(p->name, p->body.defined->name)) {
							jlog("%s", p->name);
						}
						else {
							jlog("%s[%s]", p->name, p->body.defined->name);
						}
						if (r->am->hmminfo->multipath) {
							if (align->is_iwsp[i]) {
								jlog(" #%d (sp)\n", align->loc[i]);
							}
							else {
								jlog(" #%d\n", align->loc[i]);
							}
						}
						else {
							jlog(" #%d\n", align->loc[i]);
						}
						break;
					}
				}

				jlog("re-computed AM score: %f\n", align->allscore);

				jlog("=== end forced alignment ===\n");
			}
		}
	}

	output_result(result_string, (int)strlen(result_string));
	jlog_flush();
}

boolean adin_unity_standby(int sfreq, void *dummy)
{
	debug_log("mic_standby");
	return TRUE;
}

static boolean adin_unity_open(char *arg)
{
	debug_log("mic_open");
	return TRUE;
}

boolean adin_unity_begin(char *arg)
{
	debug_log("mic_begin");
	return TRUE;
}

boolean adin_unity_end()
{
	debug_log("mic_end");
	return TRUE;
}

int adin_unity_read(SP16 *buf, int sampnum)
{
	int len = 0;
	SP16* result = audio_read_callback(sampnum, &len);
	if (len > 0)
	{
		memcpy(buf, result, len * sizeof(SP16));
	}
	return len;
}

boolean adin_unity_pause()
{
	return TRUE;
}

boolean adin_unity_terminate()
{
	return TRUE;
}

boolean adin_unity_resume()
{
	return TRUE;
}

char *adin_unity_input_name()
{
	return "Unity Microphone";
}

bool create_engine()
{
	ADIn* ad_in;

	jconf->input.type = INPUT_WAVEFORM;
	jconf->input.speech_input = SP_MIC;
	jconf->decodeopt.realtime_flag = TRUE;

	recog = j_create_instance_from_jconf(jconf);
	if (recog == NULL)
	{
		debug_log("failed to create recog instance");
		return FALSE;
	}

	// Register callbacks
	callback_add(recog, CALLBACK_EVENT_PROCESS_ONLINE, on_process_online, NULL);
	callback_add(recog, CALLBACK_EVENT_PROCESS_OFFLINE, on_process_offline, NULL);
	callback_add(recog, CALLBACK_EVENT_SPEECH_READY, on_speech_ready, NULL);
	callback_add(recog, CALLBACK_EVENT_SPEECH_START, on_speech_start, NULL);
	callback_add(recog, CALLBACK_EVENT_SPEECH_STOP, on_speech_stop, NULL);
	callback_add(recog, CALLBACK_EVENT_RECOGNITION_BEGIN, on_recognition_begin, NULL);
	callback_add(recog, CALLBACK_EVENT_RECOGNITION_END, on_recognition_end, NULL);
	callback_add(recog, CALLBACK_EVENT_PASS1_FRAME, on_pass1_frame, NULL);
	callback_add(recog, CALLBACK_EVENT_PAUSE, on_pause, NULL);
	callback_add(recog, CALLBACK_EVENT_RESUME, on_resume, NULL);
	callback_add(recog, CALLBACK_RESULT, on_result, NULL);
	callback_add(recog, CALLBACK_PAUSE_FUNCTION, on_pause_function, NULL);

	if (!j_adin_init(recog))
	{
		debug_log("failed to initialize audio in");
		return FALSE;
	}

	if (audio_read_callback != NULL)
	{
		recog->adin->ad_standby = adin_unity_standby;
		recog->adin->ad_begin = adin_unity_begin;
		recog->adin->ad_end = adin_unity_end;
		recog->adin->ad_input_name = adin_unity_input_name;
		recog->adin->ad_read = adin_unity_read;
		recog->adin->ad_pause = adin_unity_pause;
		recog->adin->ad_terminate = adin_unity_terminate;
		recog->adin->ad_resume = adin_unity_resume;
	}

	j_recog_info(recog);

	if (j_open_stream(recog, NULL) < 0)
	{
		debug_log("failed to open stream");
		return FALSE;
	}

	return TRUE;
}

