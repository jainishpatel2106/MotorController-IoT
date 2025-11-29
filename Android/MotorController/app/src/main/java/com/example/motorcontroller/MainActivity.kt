package com.example.motorcontroller

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import java.io.BufferedWriter
import java.io.OutputStreamWriter
import java.io.PrintWriter
import java.net.Socket

class MainActivity : AppCompatActivity() {

    private lateinit var ipEditText: EditText
    private lateinit var portEditText: EditText
    private lateinit var connectButton: Button
    private lateinit var speedLabel: TextView
    private lateinit var minusButton: Button
    private lateinit var plusButton: Button
    private lateinit var speedSeekBar: SeekBar
    private lateinit var stopButton: Button
    private lateinit var startButton: Button
    private lateinit var fwdButton: Button
    private lateinit var revButton: Button

    private var socket: Socket? = null
    private var writer: PrintWriter? = null
    private var isConnected = false

    private var currentSpeed = 0
    private var currentDir = "fwd"

    private val handler = Handler(Looper.getMainLooper())
    private var debounceRunnable: Runnable? = null
    private val debounceDelay = 1000L // 1 second debounce like Python

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        ipEditText = findViewById(R.id.ipEditText)
        portEditText = findViewById(R.id.portEditText)
        connectButton = findViewById(R.id.connectButton)
        speedLabel = findViewById(R.id.speedLabel)
        minusButton = findViewById(R.id.minusButton)
        plusButton = findViewById(R.id.plusButton)
        speedSeekBar = findViewById(R.id.speedSeekBar)
        stopButton = findViewById(R.id.stopButton)
        startButton = findViewById(R.id.startButton)
        fwdButton = findViewById(R.id.fwdButton)
        revButton = findViewById(R.id.revButton)

        updateDirectionButtons()
        updateSpeedLabel()

        connectButton.setOnClickListener { connectToESP() }

        // Slider change
        speedSeekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                currentSpeed = progress
                updateSpeedLabel()
                debounceSend()
            }
            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })

        minusButton.setOnClickListener {
            if (currentSpeed > 0) {
                currentSpeed--
                speedSeekBar.progress = currentSpeed
                updateSpeedLabel()
                debounceSend()
            }
        }

        plusButton.setOnClickListener {
            if (currentSpeed < 100) {
                currentSpeed++
                speedSeekBar.progress = currentSpeed
                updateSpeedLabel()
                debounceSend()
            }
        }

        stopButton.setOnClickListener {
            currentSpeed = 0
            speedSeekBar.progress = 0
            updateSpeedLabel()
            cancelDebounce()
            sendImmediate()
        }

        startButton.setOnClickListener {
            currentSpeed = 40
            speedSeekBar.progress = 40
            updateSpeedLabel()
            cancelDebounce()
            sendImmediate()
        }

        fwdButton.setOnClickListener {
            currentDir = "fwd"
            updateDirectionButtons()
            cancelDebounce()
            sendImmediate()
        }

        revButton.setOnClickListener {
            currentDir = "rev"
            updateDirectionButtons()
            cancelDebounce()
            sendImmediate()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        try { writer?.close() } catch (_: Exception) {}
        try { socket?.close() } catch (_: Exception) {}
    }

    private fun updateSpeedLabel() {
        speedLabel.text = "Speed: $currentSpeed%"
    }

    private fun updateDirectionButtons() {
        if (currentDir == "fwd") {
            fwdButton.setBackgroundColor(0xFF4CAF50.toInt())
            fwdButton.setTextColor(0xFFFFFFFF.toInt())
            revButton.setBackgroundColor(0xFFE0E0E0.toInt())
            revButton.setTextColor(0xFF000000.toInt())
        } else {
            revButton.setBackgroundColor(0xFFF44336.toInt())
            revButton.setTextColor(0xFFFFFFFF.toInt())
            fwdButton.setBackgroundColor(0xFFE0E0E0.toInt())
            fwdButton.setTextColor(0xFF000000.toInt())
        }
    }

    private fun debounceSend() {
        cancelDebounce()
        debounceRunnable = Runnable { sendImmediate() }
        handler.postDelayed(debounceRunnable!!, debounceDelay)
    }

    private fun cancelDebounce() {
        debounceRunnable?.let { handler.removeCallbacks(it) }
    }

    private fun connectToESP() {
        val ip = ipEditText.text.toString().trim()
        val port = portEditText.text.toString().trim().toIntOrNull()

        if (ip.isEmpty() || port == null) {
            Toast.makeText(this, "Invalid IP or port", Toast.LENGTH_SHORT).show()
            return
        }

        Thread {
            try {
                socket = Socket(ip, port)
                writer = PrintWriter(
                    BufferedWriter(OutputStreamWriter(socket!!.getOutputStream())),
                    true
                )
                isConnected = true

                runOnUiThread {
                    Toast.makeText(this, "Connected", Toast.LENGTH_SHORT).show()
                }

            } catch (e: Exception) {
                e.printStackTrace()
                isConnected = false
                runOnUiThread {
                    Toast.makeText(this, "Connection failed", Toast.LENGTH_SHORT).show()
                }
            }
        }.start()
    }

    private fun sendImmediate() {
        if (!isConnected) {
            runOnUiThread {
                Toast.makeText(this, "Not connected", Toast.LENGTH_SHORT).show()
            }
            return
        }

        val packet = "$currentSpeed,$currentDir"

        Thread {
            try {
                writer?.print(packet)
                writer?.flush()
            } catch (e: Exception) {
                e.printStackTrace()
                runOnUiThread {
                    Toast.makeText(this, "Send failed", Toast.LENGTH_SHORT).show()
                }
            }
        }.start()
    }
}
